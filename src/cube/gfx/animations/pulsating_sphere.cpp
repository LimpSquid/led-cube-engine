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

struct pulsating_sphere :
    configurable_animation
{
    pulsating_sphere(engine_context & context);

    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    int time_ms_;
};

animation_publisher<pulsating_sphere> const publisher;

constexpr milliseconds default_pulse_time{3000ms};
constexpr double default_radius{cube::cube_size_1d / 8.0}; // 1/8 of the cube size
constexpr double default_hue{360}; // Full hue circle
constexpr int default_iters{100};

pulsating_sphere::pulsating_sphere(engine_context & context) :
    configurable_animation(context),
    time_ms_(0)
{}

void pulsating_sphere::scene_tick(milliseconds dt)
{
    time_ms_ += static_cast<int>(dt.count());
}

void pulsating_sphere::paint(graphics_device & device)
{
    painter p(device);

    // Read properties
    auto const pulse_time = read_property<milliseconds>("pulse_time_ms");
    auto const radius = read_property<double>("radius");
    auto const iters = read_property<int>("iters");
    auto const hue = read_property<double>("hue");

    // Calculate pulsating factor
    double pulse_factor = (1.5 + sin(2.0 * M_PI * time_ms_ / static_cast<double>(pulse_time.count())));
    double current_radius = radius * (1.0 + pulse_factor);

    // Loop over sphere parameters
    for (int i = 0; i < iters; ++i) {
        double theta = M_PI * i / (iters - 1); // Polar angle
        for (int j = 0; j < iters; ++j) {
            double phi = 2.0 * M_PI * j / (iters - 1); // Azimuthal angle

            // Sphere point in local coords
            double x = current_radius * sin(theta) * cos(phi);
            double y = current_radius * sin(theta) * sin(phi);
            double z = current_radius * cos(theta);

            // Drifting hue
            double h = std::fmod(hue * (i + j) / (iters * 2) + 0.5 * time_ms_ * 0.001, 360);

            // Convert to voxel coords
            auto xi = int(std::round(x + (cube::cube_axis_max_value >> 1)));
            auto yi = int(std::round(y + (cube::cube_axis_max_value >> 1)));
            auto zi = int(std::round(z + (cube::cube_axis_max_value >> 1)));

            p.set_color(hsv(h, 1.0, 1.0));
            p.draw({ xi, yi, zi });
        }
    }
}

std::unordered_map<std::string, property_value_t> pulsating_sphere::extra_properties() const
{
    return {
        {"pulse_time_ms", default_pulse_time},
        {"radius", default_radius},
        {"iters", default_iters},
        {"hue", default_hue}
    };
}

} // End of namespace
