#include <cube/programs/program.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/timers.hpp>
#include <cube/core/logging.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <hal/graphics_device.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace cube::core;
using namespace cube::gfx;
using namespace std::chrono;
namespace po = boost::program_options;
namespace fs = std::filesystem;

namespace
{

std::vector<cube::programs::program_sigint_t> sigint_handlers;

render_engine & engine_instance()
{
    struct singleton
    {
        singleton()
        {
            sigint_handlers.push_back([&]() {
                LOG_INF("Stopping rendering engine");
                engine.stop();
            });
        }

        engine_context context;
        render_engine engine{context, graphics_device_factory<hal::graphics_device_t>{}};
    };

    static singleton s;
    return s.engine;
}

void handle_file(std::vector<std::string> const & args)
{
    if (!args.empty()) {
        auto & engine = engine_instance();
        animation_list_t animations;

        for (auto const & arg : args) {
            auto const filepath = fs::path(arg);
            if (!fs::exists(filepath)) {
                LOG_WRN("Ignoring non-existing file", LOG_ARG("filepath", filepath.native()));
                continue;
            }

            try {
                std::ifstream ifs(filepath.native());
                auto load_result = load_animations(nlohmann::json::parse(ifs), engine.context());

                if (!load_result)
                    throw std::runtime_error(load_result.error().what);
                animations = std::move(*load_result);
            } catch (std::exception const & ex) {
                throw std::runtime_error("In file " + filepath.native() + ": " + ex.what());
            }
        }

        LOG_INF("Playing animations from file", LOG_ARG("number_of_animations", animations.size()));

        if (!animations.empty()) {
            std::size_t index = 0;
            single_shot_timer player(engine.context(), [&](auto, auto) {
                auto const & [name, animation] = animations[index];
                index = (index + 1) % animations.size();
                engine.load(std::static_pointer_cast<cube::core::animation>(animation));
                player.start(animation->get_duration());

                LOG_INF("Playing animation",
                    LOG_ARG("animation", name),
                    LOG_ARG("label", animation->get_label()),
                    LOG_ARG("duration_ms", animation->get_duration().count()));
            });
            player.start(0ms);
            engine.run();
        }

        std::exit(EXIT_SUCCESS);
    }

    std::cout
        << "Usage: led-cube-engine render --file <filepath> [filepath...]\n\n"
        << "Examples:\n"
        << "  led-cube-engine render --file  animations.json\n"
        << "  led-cube-engine render --file ../relative/path/to/animations.json /absolute/path/to/animations.json\n";
    std::exit(EXIT_FAILURE);
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

        engine.load(std::static_pointer_cast<cube::core::animation>(*animation));
        engine.run();
        std::exit(EXIT_SUCCESS);
    }

    std::cout
        << "Usage: led-cube-engine render --animation <name> [properties]\n\n"
        << "Examples:\n"
        << "  led-cube-engine render --animation helix\n"
        << "  led-cube-engine render --animation lightning '{\"cloud_gradient\":{\"gradient_stops\":[{\"stop_color\":\"orange\",\"stop_position\":0.5},{\"stop_color\":\"white\",\"stop_position\":1}]}}'\n";
    std::exit(EXIT_FAILURE);
}

} // End of namespace

namespace cube::programs
{

program const program_render
{
    "render",
    "render animations to the LED cube engine's graphics device",
    [](int ac, char const * const av[]) -> int
    {
        po::options_description desc("Available options");
        desc.add_options()
            ("help,h", "produce a help message")
            ("file,f", po::value<std::vector<std::string>>()
                ->zero_tokens()
                ->multitoken()
                ->notifier(handle_file), "render animations from one or multiple files")
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
            << desc;
        return EXIT_FAILURE;
    },
    []()
    {
        std::for_each(sigint_handlers.begin(), sigint_handlers.end(), [](auto h) { h(); });
    }
};


} // End of namespace
