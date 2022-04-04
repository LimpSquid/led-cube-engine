#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/timers.hpp>
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


std::vector<std::function<void()>> sigint_handlers;

engine & engine_instance()
{
    struct engine_singleton
    {
        engine_singleton()
        {
            sigint_handlers.push_back([&]() { instance.stop(); });
        }

        engine_context context;
        engine instance{context, graphics_device_factory<hal::graphics_device_t>{}};
    };

    static engine_singleton s;
    return s.instance;
}

void handle_file(std::vector<std::string> const & args)
{
    if (!args.empty()) {
        auto & engine = engine_instance();
        std::vector<animation_pointer_t> animations;

        for (auto const & arg : args) {
            auto const filepath = fs::path(arg);
            if (!fs::exists(filepath)) {
                std::cout << "Ignoring non-existing file: " + filepath.native() << '\n';
                continue;
            }

            try {
                std::ifstream ifs(filepath.native());
                auto file_animations = load_animations(nlohmann::json::parse(ifs), engine.context());

                if (!file_animations)
                    throw std::runtime_error(file_animations.error().what);
                for (auto & animation : *file_animations)
                    animations.push_back(std::move(animation));
            } catch (std::exception const & ex) {
                throw std::runtime_error("In file " + filepath.native() + ": " + ex.what());
            }
        }

        std::cout << "Found " << animations.size() << " animations in file(s)\n";

        if (!animations.empty()) {
            std::size_t index = 0;
            single_shot_timer player(engine.context(), [&](auto, auto) {
                auto & animation = animations[index];
                index = (index + 1) % animations.size();
                engine.load(std::static_pointer_cast<cube::core::animation>(animation));
                player.start(animation->get_duration());
            });
            player.start(0ms);
            engine.run();
        }

        std::exit(EXIT_SUCCESS);
    }

    std::cout
        << "Usage: led-cube-engine render --file <file> [file...]\n\n"
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

void sigint_render()
{
    for (auto const & handler : sigint_handlers)
        handler();
}

int main_render(int ac, char const * const av[])
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
}

} // End of namespace
