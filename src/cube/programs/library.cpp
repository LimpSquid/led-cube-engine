#include <cube/programs/program.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace cube::core;
using namespace cube::gfx;
using namespace cube::programs;
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
        for (auto const & animation : animations)
            std::cout << "  - " << animation << '\n';
    }

    std::exit(EXIT_SUCCESS);
}

void handle_info(std::vector<std::string> const & args)
{
    if (!args.empty()) {
        auto const animations = std::set<std::string>(args.begin(), args.end());
        engine_context context{};

        nlohmann::json json = nlohmann::json::array();

        for (auto it = animations.begin(); it != animations.end(); ++it) {
            auto animation_name = *it;
            auto incubated = library::instance().incubate(animation_name, context);

            if (incubated) {
                nlohmann::json item = nlohmann::json::object();
                item.emplace(make_field("animation", animation_name));
                item.emplace(make_field("properties", (*incubated)->dump_properties()));
                json.push_back(std::move(item));
            }
        }

        std::cout << json.dump(2) << '\n';

        std::exit(EXIT_SUCCESS);
    }

    std::cout
        << "Usage: led-cube-engine library --info <name> [name...]\n\n"
        << "Examples:\n"
        << "  led-cube-engine library --info helix\n"
        << "  led-cube-engine library --info helix stars\n";
    std::exit(EXIT_FAILURE);
}

void handle_info_all()
{
    engine_context context{};
    auto & lib = library::instance();

    nlohmann::json json = nlohmann::json::array();

    for (auto const & animation_name : lib.available_animations()) {
        auto incubated = library::instance().incubate(animation_name, context);

        if (incubated) {
            nlohmann::json item = nlohmann::json::object();
            item.emplace(make_field("animation", animation_name));
            item.emplace(make_field("properties", (*incubated)->dump_properties()));
            json.push_back(std::move(item));
        }
    }

    std::cout << json.dump(2) << '\n';

    std::exit(EXIT_SUCCESS);
}

void handle_dump_properties(std::vector<std::string> const & args)
{
    if (args.size() == 2) {
        engine_context context{};
        auto incubated = library::instance().incubate(args.at(0), context);

        if (!incubated)
            std::runtime_error(incubated.error().what);

        auto & animation = *incubated;
        animation->load_properties(nlohmann::json::parse(args.at(1)));
        std::cout << animation->dump_properties().dump(2) << '\n';
        std::exit(EXIT_SUCCESS);
    }

    std::cout
        << "Usage: led-cube-engine library --dump-properties <name> <properties>\n\n"
        << "Example:\n"
        << "  led-cube-engine library --dump-properties helix '{\"rotation_time_ms\":7500,\"length\":4}'\n";
    std::exit(EXIT_FAILURE);
}

program const program_library
{
    "library",
    "list info about the LED cube engine's animation library",
    [](int ac, char const * const av[]) -> int
    {
        po::options_description desc("Available options");
        desc.add_options()
            ("help,h", "produce a help message")
            ("list", po::bool_switch()
                ->notifier(bool_switch_notifier(handle_list)), "list available animation(s)")
            ("info", po::value<std::vector<std::string>>()
                ->zero_tokens()
                ->multitoken()
                ->notifier(handle_info), "print info about one or more animations")
            ("info-all", po::bool_switch()
                ->notifier(bool_switch_notifier(handle_info_all)), "print info about all available animations")
            ("dump-properties", po::value<std::vector<std::string>>()
                ->zero_tokens()
                ->multitoken()
                ->notifier(handle_dump_properties), "load an animation and dump its resulting properties");

        po::variables_map cli_variables;
        po::store(po::parse_command_line(ac, av, desc), cli_variables);
        po::notify(cli_variables);

        // Print help if no handler exited
        std::cout
            << "Usage: led-cube-engine library <option> [arg...]\n\n"
            << desc;
        return EXIT_FAILURE;
    }
};

} // End of namespace
