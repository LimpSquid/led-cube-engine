#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/gfx/library.hpp>
#include <hal/graphics_device.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace cube::core;
using namespace cube::gfx;
using namespace std::chrono;
namespace po = boost::program_options;

namespace
{

engine & engine_instance()
{
    static engine_context context;
    static engine instance(context, graphics_device_factory<hal::graphics_device_t>{});
    return instance;
}

void handle_animation(std::vector<std::string> const & args)
{
    if (args.size() > 0 && args.size() <= 2) {
        auto & engine = engine_instance();
        auto animation = library::instance().incubate(args[0], engine.context());

        if (!animation)
            throw std::runtime_error(animation.error().what);
        if (args.size() == 2)
            (*animation)->load_properties(nlohmann::json::parse(args[1]));

        engine.load(animation->get());
        engine.run(); // Does not return
    }

    std::cout
        << "Usage: led-cube-engine render --animation <name> [properties]\n\n"
        << "Examples:\n"
        << "  led-cube-engine render --animation helix\n"
        << "  led-cube-engine render --animation lightning '{\"cloud_color\": {\"name\": \"orange\"}}'\n";
    std::exit(EXIT_FAILURE);
}

} // End of namespace

namespace cube::programs
{

int main_render(int ac, char const * const av[])
{
    po::options_description desc("Available options");
    desc.add_options()
        ("help,h", "produce a help message")
        ("file,f", "render animations from file")
        ("animation,", po::value<std::vector<std::string>>()
            ->zero_tokens()
            ->multitoken()
            ->notifier(handle_animation), "render an animation");

    po::variables_map cli_variables;
    po::store(po::parse_command_line(ac, av, desc), cli_variables);
    po::notify(cli_variables);

    // Print help if no handler exited
    std::cout
        << "Usage: led-cube-engine render <option> [arg...]\n\n"
        << desc << '\n';
    return EXIT_FAILURE;
}

} // End of namespace
