// NB: Created by ChatGPT

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct wave_surface :
    configurable_animation
{
    wave_surface(engine_context & context);

    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    int time_ms_;
};

animation_publisher<wave_surface> const publisher;

constexpr milliseconds default_wave_time{3000ms};
constexpr int default_iters{300}; // Number of points along each axis
constexpr double default_amplitude{cube::cube_size_1d / 4.0}; // 1/4 of the cube size
constexpr double default_frequency{0.2}; // Frequency of the wave

wave_surface::wave_surface(engine_context & context) :
    configurable_animation(context),
    time_ms_(0)
{}

void wave_surface::scene_tick(milliseconds dt)
{
    time_ms_ += static_cast<int>(dt.count());
}

void wave_surface::paint(graphics_device & device)
{
    painter p(device);

    // Read properties
    auto const wave_time = read_property<milliseconds>("wave_time_ms");
    auto const iters = read_property<int>("iters");
    auto const amplitude = read_property<double>("amplitude");
    auto const frequency = read_property<double>("frequency");

    // Calculate the wave offset based on time
    double wave_offset = time_ms_ / static_cast<double>(wave_time.count()) * 2.0 * M_PI; // Full wave cycle

    // Loop over wave parameters to create a grid
    for (int i = 0; i < iters; ++i) {
        for (int j = 0; j < iters; ++j) {
            // Spread points along the X and Y axes
            double x = (i - iters / 2.0) * (cube::cube_size_1d / (iters / 2.0)); // X position
            double y = (j - iters / 2.0) * (cube::cube_size_1d / (iters / 2.0)); // Y position

            // Calculate Z position using a combination of sine and cosine for a more complex wave
            double z = amplitude * (sin(frequency * (x + wave_offset)) + cos(frequency * (y + wave_offset)));

            // Drifting hue
            double h = fmod(360.0 * (i + j) / (iters * 2) + 0.5 * time_ms_ * 0.1, 360);
            double s = 0.75 + 0.25 * sin(time_ms_ * 0.001 + (i + j));

            // Convert to voxel coords
            auto xi = int(std::round(x + (cube::cube_axis_max_value >> 1)));
            auto yi = int(std::round(y + (cube::cube_axis_max_value >> 1)));
            auto zi = int(std::round(z + (cube::cube_axis_max_value >> 1)));

            // Draw the point at the calculated position
            p.set_color(hsv(h, s, 0.75));
            p.draw({ xi, yi, zi }); // Use draw method to plot the point
        }
    }
}

std::unordered_map<std::string, property_value_t> wave_surface::extra_properties() const
{
    return {
        {"wave_time_ms", default_wave_time},
        {"iters", default_iters},
        {"amplitude", default_amplitude},
        {"frequency", default_frequency}
    };
}

} // End of namespace
