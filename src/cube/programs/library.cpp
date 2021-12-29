#include <cube/core/engine_context.hpp>
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

po::variables_map cli_options;

void handle_list()
{
    auto const animations = library::instance().available_animations();

    if (animations.empty())
        std::cout << "No animations available.\n";
    else {
        std::cout << "Available animations:\n";
        for (auto const animation : animations)
            std::cout << "  - " << animation << '\n';
    }

    std::exit(EXIT_SUCCESS);
}

void handle_info(std::vector<std::string> const & animations)
{
    auto const unique_animations = std::set<std::string>(animations.begin(), animations.end());
    engine_context context{};

    for (auto it = unique_animations.begin(); it != unique_animations.end(); ++it) {
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
        if (std::distance(it, unique_animations.end()) > 1)
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
        ("help", "produce help message")
        ("list", po::bool_switch()->notifier(bool_switch_notifier(handle_list)), "list available animations")
        ("info", po::value<std::vector<std::string>>()->multitoken()->notifier(handle_info), "print info about one or more animations");

    po::store(po::parse_command_line(ac, av, desc), cli_options);
    po::notify(cli_options);

    // Print help if no handler exited
    std::cout
        << "Usage: led-cube-engine library <option> [arg...]\n\n"
        << desc << '\n';
    return EXIT_FAILURE;
}

} // End of namespace
