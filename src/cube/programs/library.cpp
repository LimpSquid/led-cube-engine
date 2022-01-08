#include <cube/core/engine_context.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace cube::core;
using namespace cube::gfx;
namespace po = boost::program_options;

namespace
{

template<typename H>
auto bool_switch_notifier(H handler)
{
    return [h = std::move(handler)](bool run) { if (run) h(); };
}

void handle_list()
{
    auto const animations = library::instance().available_animations();

    if (animations.empty())
        std::cout << "No animations available.\n";
    else {
        std::cout << "Available animation(s):\n";
        for (auto const animation : animations)
            std::cout << "  - " << animation << '\n';
    }

    std::exit(EXIT_SUCCESS);
}

void handle_info(std::vector<std::string> const & args)
{
    auto const animations = std::set<std::string>(args.begin(), args.end());
    engine_context context{};

    for (auto it = animations.begin(); it != animations.end(); ++it) {
        auto incubated = library::instance().incubate(*it, context);

        if (incubated) {
            std::cout
                << "Default properties for '" << *it << "':\n"
                << (*incubated)->dump_properties().dump(2) << '\n';
        } else {
            std::cout
                << "Error for '" << *it << "': "
                << incubated.error().what << '\n';
        }

        // Separator
        if (std::distance(it, animations.end()) > 1)
            std::cout << "=====\n\n";
    }

    std::exit(EXIT_SUCCESS);
}

}

namespace cube::programs
{

int main_library(int ac, char const * const av[])
{
    po::options_description desc("Available options");
    desc.add_options()
        ("help,h", "produce a help message")
        ("list", po::bool_switch()
            ->notifier(bool_switch_notifier(handle_list)), "list available animation(s)")
        ("info", po::value<std::vector<std::string>>()
            ->multitoken()
            ->notifier(handle_info), "print info about one or more animations");

    po::variables_map cli_variables;
    po::store(po::parse_command_line(ac, av, desc), cli_variables);
    po::notify(cli_variables);

    // Print help if no handler exited
    std::cout
        << "Usage: led-cube-engine library <option> [arg...]\n\n"
        << desc << '\n';
    return EXIT_FAILURE;
}

} // End of namespace
