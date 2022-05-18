#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <hal/rpi_cube/bus_proto.hpp>
#include <hal/rpi_cube/bus_flasher.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/logging.hpp>
#include <cube/programs/program.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace cube::core;
using namespace cube::programs;
using namespace std::chrono;
namespace po = boost::program_options;

namespace
{

std::vector<program_sigint_t> sigint_handlers;

struct hexflash_binding
{
    poll_engine & engine;
    hal::rpi_cube::resources & resources;
    hal::rpi_cube::bus_comm & bus_comm;
};

hexflash_binding hexflash_instance()
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
    return hexflash_binding{s.engine, s.resources, s.bus_comm};
}

template<typename H>
auto bool_switch_notifier(H handler)
{
    return [h = std::move(handler)](bool run) { if (run) h(); };
}

void handle_detect_boards()
{
    using namespace hal::rpi_cube;

    auto [engine, resources, bus_comm] = hexflash_instance();

    // TODO: eventually also detect boards that are already in bootloader mode
    bus_comm.send_for_all<bus_command::app_get_version>({}, [&](auto responses) {
        for (auto const & [slave, response] : responses) {
            if (!response) {
                LOG_PLAIN("Slave not found", LOG_ARG("address", as_hex(slave)));
                continue;
            }

            std::string version =
                "v" + std::to_string(response->major) +
                "." + std::to_string(response->minor) +
                "." + std::to_string(response->patch);

            LOG_PLAIN("Detected slave",
                LOG_ARG("address", as_hex(slave)),
                LOG_ARG("version", version));
        }
        engine.stop();
    }, resources.bus_comm_slave_addresses);

    engine.run();
    std::exit(EXIT_SUCCESS);
}

void handle_reset_boards()
{
    constexpr milliseconds reset_delay{250}; // TODO: when led controller firmware is fixed to only reset after the bus is idle, set this to zero

    using namespace hal::rpi_cube;

    auto [engine, resources, bus_comm] = hexflash_instance();
    bus_request_params<bus_command::app_exe_cpu_reset> params;
    params.delay_ms = reset_delay.count();

    bus_comm.send_for_all(std::move(params), [&](auto responses) {
        for (auto const & [slave, response] : responses) {
            if (response)
                LOG_PLAIN("Slave reset", LOG_ARG("address", as_hex(slave)));
            else
                LOG_PLAIN("Slave not found", LOG_ARG("address", as_hex(slave)));
        }
        engine.stop();
    }, resources.bus_comm_slave_addresses);

    engine.run();
    std::exit(EXIT_SUCCESS);
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

void handle_flash_boards()
{
    using namespace hal::rpi_cube;

    auto [engine, resources, bus_comm] = hexflash_instance();
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
            ("flash-boards", po::bool_switch() // TODO: args
                ->notifier(bool_switch_notifier(handle_flash_boards)), "flash all available boards on the bus");

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
