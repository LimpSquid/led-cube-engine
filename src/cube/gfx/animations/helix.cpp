#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct helix :
    configurable_animation
{
    helix(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    gradient gradient_;
    std::optional<ease_in_sine> fade_in_;
    std::optional<ease_out_sine> fade_out_;
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
    configurable_animation(context)
{ }

void helix::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            auto const rotation_time = read_property<milliseconds>("rotation_time_ms");

            gradient_ = read_property<gradient>("gradient");
            thickness_ = read_property<int>("thickness");
            length_ =  2.0 * M_PI * read_property<double>("length");
            omega_ = (2.0 * M_PI) / static_cast<double>(rotation_time.count());
            time_ = rand(range{0, UINT16_MAX});

            fade_in_.emplace(context(), easing_config{{0.0, 1.0}, 50, get_transition_time()});
            fade_out_.emplace(context(), easing_config{{1.0, 0.0}, 50, get_transition_time()});

            fade_in_->start();
            break;
        }
        case animation_state::stopping:
            fade_out_->start();
            break;
        case animation_state::stopped:
            fade_in_->stop();
            fade_out_->stop();
            break;
    }
}

void helix::scene_tick(milliseconds dt)
{
    time_ += static_cast<int>(dt.count());
}

void helix::paint(graphics_device & device)
{
    painter p(device);

    auto const phase_shift_sin_factor = read_property<double>("phase_shift_sin_factor");
    auto const phase_shift_cos_factor = read_property<double>("phase_shift_cos_factor");

    for (int y = 0; y < cube::cube_size_1d; y++) {
        double const phase_shift = map(y, cube_axis_range, range(0.0, length_));
        double const x1 = std::sin(time_ * omega_ + phase_shift * std::cos(time_ * omega_ * phase_shift_sin_factor));
        double const z1 = std::cos(time_ * omega_ + phase_shift * std::cos(time_ * omega_ * phase_shift_cos_factor));
        int const x = map(x1, unit_circle_range, cube_axis_range);
        int const z = map(z1, unit_circle_range, cube_axis_range);

        auto c = gradient_(map_sin(time_ * omega_ + 0.5 * phase_shift)).vec();
        c *= rgb_vec(fade_in_->value());
        c *= rgb_vec(fade_out_->value());

        p.set_color(c);
        p.sphere({x, y, z}, thickness_);
    }
}

std::unordered_map<std::string, property_value_t> helix::extra_properties() const
{
    return {
        {"rotation_time_ms", default_rotation_time},
        {"phase_shift_cos_factor", default_phase_shift_factor},
        {"phase_shift_sin_factor", default_phase_shift_factor},
        {"thickness", default_thickness},
        {"length", default_length},
        {"gradient", default_gradient},
    };
}

} // End of namespace
