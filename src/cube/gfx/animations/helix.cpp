#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

struct helix :
    configurable_animation
{
    helix(engine_context & context);

    void start() override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    void stop() override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    gradient gradient_;
    ease_in_sine fader_;
    int time_;
    int thickness_;
    double omega_;
    double length_;
};

animation_publisher<helix> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr milliseconds default_rotation_time{1500ms};
constexpr int default_thickness{3};
constexpr double default_length{0.875};
constexpr double default_phase_shift_factor{0.0};
gradient const default_gradient
{
    {0.00, color_cyan},
    {0.50, color_yellow},
    {1.00, color_magenta},
};

helix::helix(engine_context & context) :
    configurable_animation(context),
    fader_(context, {{0.1, 1.0}, 10, 1000ms})
{ }

void helix::start()
{
    auto const rotation_time = read_property<milliseconds>("helix_rotation_time_ms");

    gradient_ = read_property<gradient>("helix_gradient");
    thickness_ = read_property<int>("helix_thickness");
    length_ =  2.0 * M_PI * read_property<double>("helix_length");
    omega_ = (2.0 * M_PI) / static_cast<double>(rotation_time.count());
    time_ = rand(range{0, UINT16_MAX});

    fader_.start();
}

void helix::scene_tick(milliseconds dt)
{
    time_ += static_cast<int>(dt.count());
}

void helix::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    auto const phase_shift_sin_factor = read_property<double>("helix_phase_shift_sin_factor");
    auto const phase_shift_cos_factor = read_property<double>("helix_phase_shift_cos_factor");

    for (int y = 0; y < cube::cube_size_1d; y++) {
        double const phase_shift = map(y, cube_axis_range, range(0.0, length_));
        double const x1 = std::sin(time_ * omega_ + phase_shift * std::cos(time_ * omega_ * phase_shift_sin_factor));
        double const z1 = std::cos(time_ * omega_ + phase_shift * std::cos(time_ * omega_ * phase_shift_cos_factor));
        int const x = map(x1, unit_circle_range, cube_axis_range);
        int const z = map(z1, unit_circle_range, cube_axis_range);

        p.set_color(gradient_(abs_sin(time_ * omega_ + 0.5 * phase_shift)).vec() * rgb_vec(fader_.value()));
        p.sphere({x, y, z}, thickness_);
    }
}

void helix::stop()
{
    fader_.stop();
}

std::unordered_map<std::string, property_value_t> helix::extra_properties() const
{
    return {
        { "helix_rotation_time_ms", default_rotation_time },
        { "helix_phase_shift_cos_factor", default_phase_shift_factor },
        { "helix_phase_shift_sin_factor", default_phase_shift_factor },
        { "helix_thickness", default_thickness },
        { "helix_length", default_length },
        { "helix_gradient", default_gradient },
    };
}

} // End of namespace
