#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/timers.hpp>
#include <cube/core/json_util.hpp>
#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/gfx/animations/stars.hpp>
#include <cube/gfx/animations/helix.hpp>
#include <cube/gfx/library.hpp>
#include <hal/graphics_device.hpp>
#include <chrono>
#include <iostream>

using namespace cube::core;
using namespace cube::gfx;
using namespace std::chrono;

namespace
{

auto const animation_definition = R"(
[
    {
        "animation": "lightning",
        "enabled": true
    },
    {
        "animation": "helix",
        "enabled": true,
        "properties": {
            "animation_label": "normal helix",
            "helix_rotation_time_ms": 1250,
            "helix_phase_shift_sin_factor": 0.02,
            "helix_phase_shift_cos_factor": 0.01,
            "color_gradient_start": {
                "red": 255,
                "green": 0,
                "blue": 0
            },
            "color_gradient_end": {
                "red": 0,
                "green": 0,
                "blue": 255
            }
        }
    },
    {
        "animation": "stars",
        "enabled": true
    },
    {
        "animation": "helix",
        "enabled": false,
        "properties": {
            "animation_label": "fat helix",
            "helix_phase_shift_sin_factor": 0.02,
            "helix_phase_shift_cos_factor": 0.01,
            "helix_thickness": 10.0,
            "color_gradient_start": { "name": "orange" },
            "color_gradient_end": { "name": "green" }
        }
    },
    {
        "animation": "helix",
        "properties": {
            "animation_label": "long helix",
            "helix_rotation_time_ms": 1500,
            "helix_phase_shift_sin_factor": 0.04,
            "helix_thickness": 1.5,
            "helix_length": 4.0,
            "color_gradient_start": { "name": "magenta" },
            "color_gradient_end": { "name": "cyan" }
        }
    }
]
)"_json;

} // End of namespace

namespace cube::programs
{

int main_render(int ac, char const * const av[])
{
    engine_context context;
    engine cube_engine(context, graphics_device_factory<hal::graphics_device_t>{});

    auto animations = load_animations(animation_definition, context);
    if (!animations)
        throw std::runtime_error("Failed to load animations: " + animations.error().what);

    int animations_index = 0;
    recurring_timer timer(context, [&](auto, auto) {
        auto * animation = (*animations)[animations_index++ % animations->size()].get();
        cube_engine.load(animation);
    });
    timer.start(20s, true);
    cube_engine.run(); // Todo: eventually we need to cycle through animations

    return EXIT_SUCCESS;
}

} // End of namespace
