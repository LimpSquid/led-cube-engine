#include <driver/rpi_cube/resources.hpp>
#include <driver/rpi_cube/bus_comm.hpp>
#include <driver/rpi_cube/bus_proto.hpp>
#include <driver/rpi_cube/bus_flasher.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/logging.hpp>
#include <cube/core/composite_function.hpp>
#include <cube/programs/program.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace cube::core;
using namespace cube::programs;
using namespace std::chrono;
using std::operator""s;
namespace po = boost::program_options;

namespace
{

std::vector<program_sigint_t> sigint_handlers;

struct bus_transferring
{
    driver::rpi_cube::bus_comm & bus;

    bool operator()()
    {
        return bus.state() == driver::rpi_cube::bus_state::transfer;
    }
};

struct binding
{
    poll_engine & engine;
    driver::rpi_cube::resources & resources;
    driver::rpi_cube::bus_comm & bus_comm;
};

binding instance()
{
    struct singleton
    {
        singleton()
        {
            sigint_handlers.push_back([&]() { engine.stop(); });
        }

        engine_context context;
        poll_engine engine{context};
        driver::rpi_cube::resources resources{context};
        driver::rpi_cube::bus_comm bus_comm{resources.bus_comm_device};
    };

    static singleton s;
    return binding{s.engine, s.resources, s.bus_comm};
}

template<typename H>
auto bool_switch_notifier(H handler)
{
    return [h = std::move(handler)](bool run) { if (run) h(); };
}

void handle_detect_boards()
{
    using namespace driver::rpi_cube;

    auto [engine, resources, bus_comm] = instance();

    for (auto const & slave : resources.bus_comm_slave_addresses) {
        auto [bl_handler, app_handler] = decompose_function([slave](
            bus_response_params_or_error<bus_command::bl_get_version> bl_response,
            bus_response_params_or_error<bus_command::app_get_version> app_response) {
                if (!bl_response && !app_response) {
                    LOG_PLAIN("Slave not found", LOG_ARG("address", as_hex(slave)));
                    return;
                }

                std::string version =
                    "v" + std::to_string(app_response ? app_response->major : bl_response->major) +
                    "." + std::to_string(app_response ? app_response->minor : bl_response->minor) +
                    "." + std::to_string(app_response ? app_response->patch : bl_response->patch);

                LOG_PLAIN("Detected slave in "s + (app_response ? "app" : "bootload") + " mode",
                    LOG_ARG("address", as_hex(slave)),
                    LOG_ARG("version", version));
            }
        );

        bus_comm.send<bus_command::bl_get_version>({}, slave, std::move(bl_handler));
        bus_comm.send<bus_command::app_get_version>({}, slave, std::move(app_handler));
    }

    engine.run_while(bus_transferring{bus_comm});
    std::exit(bus_comm.state() == bus_state::idle
        ? EXIT_SUCCESS
        : EXIT_FAILURE);
}

void handle_reset_boards()
{
    using namespace driver::rpi_cube;

    auto [engine, resources, bus_comm] = instance();

    // Reset Immediately
    bus_comm.send_for_all<bus_command::app_exe_cpu_reset>({}, [&](auto responses) {
        for (auto const & [slave, response] : responses) {
            if (response)
                LOG_PLAIN("Slave reset", LOG_ARG("address", as_hex(slave)));
            else
                LOG_PLAIN("Slave not found", LOG_ARG("address", as_hex(slave)));
        }
    }, resources.bus_comm_slave_addresses);

    engine.run_while(bus_transferring{bus_comm});
    std::exit(bus_comm.state() == bus_state::idle
        ? EXIT_SUCCESS
        : EXIT_FAILURE);
}

void handle_dump_blob(std::vector<std::string> const & args)
{
    using namespace driver::rpi_cube;

    if (args.size() == 3) {
        memory_layout layout;

        try {
            layout.start_address = static_cast<uint32_t>(std::stoul(args[1], nullptr, 16));
            layout.end_address = layout.start_address + static_cast<uint32_t>(std::stoul(args[2], nullptr, 16));
        } catch(...) {
            throw std::runtime_error("Unable to parse number input");
        }

        auto blob = parse_hex_file(args[0], layout);
        if (!blob)
            throw std::runtime_error(blob.error().what);

        for (auto const data : *blob)
            std::cout << data;
        std::exit(EXIT_SUCCESS);
    }

    std::cout
        << "Usage: led-cube-engine hexflash --dump-blob <filepath> <start_address> <size>\n\n"
        << "Examples:\n"
        << "  led-cube-engine hexflash --dump-blob /tmp/board.hex 0x1d000000 0xffff\n";
    std::exit(EXIT_FAILURE);
}

void handle_flash_boards(std::vector<std::string> const & args)
{
    using namespace driver::rpi_cube;

    constexpr unsigned int busy_indicator_length{5};

    if (args.size() == 1) {
        auto [engine, _, bus_comm] = instance();

        unsigned int n_dots = 0;
        recurring_timer busy_indicator{engine.context(), [&](auto, auto) {
            if (n_dots == busy_indicator_length) {
                std::cout << '\r' << std::string(busy_indicator_length, ' ') << '\r' << std::flush;
                n_dots = 0;
            }

            std::cout << '.' << std::flush;
            n_dots++;
        }, true};

        bus_flasher flasher{bus_comm, [&](auto result) {
            if (busy_indicator.is_running()) {
                busy_indicator.stop();
                std::cout << '\r' << std::string(busy_indicator_length, ' ') << '\r' << std::flush;
            }
            if (!result)
                throw std::runtime_error(result.error().what);

            for (auto && node : result->flashed_nodes)
                LOG_PLAIN("Succesfully flashed slave", LOG_ARG("address", as_hex(node.address)));
            for (auto && node : result->failed_nodes)
                LOG_PLAIN("Failed to flash slave",
                    LOG_ARG("address", as_hex(node.first.address)),
                    LOG_ARG("reason", node.second));
        }};
        flasher.flash_hex_file(args[0]);

        LOG_PLAIN("Flashing hex file", LOG_ARG("filepath", args[0]));

        if (get_runtime_log_level() != log_prio::debug)
            busy_indicator.start(500ms);

        engine.run_while(bus_transferring{bus_comm});
        std::exit(bus_comm.state() == bus_state::idle
            ? EXIT_SUCCESS
            : EXIT_FAILURE);
    }

    std::cout
        << "Usage: led-cube-engine hexflash --flash-boards <filepath>\n\n"
        << "Examples:\n"
        << "  led-cube-engine hexflash --flash-boards /tmp/board.hex\n";
    std::exit(EXIT_FAILURE);
}

} // End of namespace

namespace driver::rpi_cube
{

program const program_hexflash
{
    "hexflash",
    "flash Intel HEX files to the LED cube controller boards.",
    [](int ac, char const * const av[]) -> int
    {
        po::options_description desc("Available options");
        desc.add_options()
            ("help,h", "produce a help message")
            ("detect-boards", po::bool_switch()
                ->notifier(bool_switch_notifier(handle_detect_boards)), "detect all boards on the bus")
            ("reset-boards", po::bool_switch()
                ->notifier(bool_switch_notifier(handle_reset_boards)), "reset all boards on the bus")
            ("dump-blob", po::value<std::vector<std::string>>()
                ->zero_tokens()
                ->multitoken()
                ->notifier(handle_dump_blob), "parse the hex file and dump the blob to stdout")
            ("flash-boards", po::value<std::vector<std::string>>()
                ->zero_tokens()
                ->multitoken()
                ->notifier(handle_flash_boards), "flash all available boards on the bus");

        po::variables_map cli_variables;
        po::store(po::parse_command_line(ac, av, desc), cli_variables);
        po::notify(cli_variables);

        // Print help if no handler exited
        std::cout
            << "Usage: led-cube-engine hexflash <option> [arg...]\n\n"
            << desc;
        return EXIT_FAILURE;
    },
    []()
    {
        for (auto && handler : sigint_handlers)
            handler();
    }
};

} // End of namespace
