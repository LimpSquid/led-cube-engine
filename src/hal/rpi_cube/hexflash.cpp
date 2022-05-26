#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <hal/rpi_cube/bus_proto.hpp>
#include <hal/rpi_cube/bus_flasher.hpp>
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
    hal::rpi_cube::bus_comm & bus;

    bool operator()()
    {
        return bus.state() == hal::rpi_cube::bus_state::transfer;
    }
};

struct binding
{
    poll_engine & engine;
    hal::rpi_cube::resources & resources;
    hal::rpi_cube::bus_comm & bus_comm;
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
        hal::rpi_cube::resources resources{context};
        hal::rpi_cube::bus_comm bus_comm{resources.bus_comm_device};
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
    using namespace hal::rpi_cube;

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
                    "." + std::to_string(app_response ? app_response->patch : bl_response->minor);

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
    constexpr milliseconds reset_delay{250}; // TODO: when led controller firmware is fixed to only reset after the bus is idle, set this to zero

    using namespace hal::rpi_cube;

    auto [engine, resources, bus_comm] = instance();
    bus_request_params<bus_command::app_exe_cpu_reset> params;
    params.delay_ms = reset_delay.count();

    bus_comm.send_for_all(std::move(params), [&](auto responses) {
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
    using namespace hal::rpi_cube;

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
    using namespace hal::rpi_cube;

    if (args.size() == 1) {
        auto [engine, _, bus_comm] = instance();
        bus_flasher flasher{bus_comm};
        flasher.flash_hex_file(args[0]);

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

namespace hal::rpi_cube
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
        std::for_each(sigint_handlers.begin(), sigint_handlers.end(), [](auto h) { h(); });
    }
};

} // End of namespace
