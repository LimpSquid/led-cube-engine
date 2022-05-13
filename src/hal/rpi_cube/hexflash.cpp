#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <hal/rpi_cube/bus_proto.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/logging.hpp>
#include <cube/programs/program.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace cube::core;
using namespace cube::programs;
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

    bus_comm.send_for_all<bus_command::get_sys_version>({}, [&](auto responses) {
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

}

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
                ->notifier(bool_switch_notifier(handle_detect_boards)), "detect all boards on the bus");

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
