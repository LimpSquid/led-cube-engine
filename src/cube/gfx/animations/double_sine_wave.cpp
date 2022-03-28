#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct double_sine_wave :
    configurable_animation
{
    PROPERTY_ENUM
    (
        wave_period,            // Number of voxels along the x-axis for one wave period
        wave_period_time_ms,    // Time in milliseconds to complete one wave period
        color_gradient_start,   // Start color of gradient
        color_gradient_end      // End color of gradient
    )

    struct wave
    {
        int time_count;
        color gradient_start;
        color gradient_end;
    };

    double_sine_wave(engine_context & context);

    void start() override;
    void paint(graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    std::array<wave, 2> waves_;
    recurring_timer update_timer_;
    ease_in_sine fader_;
    int period_;
    double omega_;
};

animation_publisher<double_sine_wave> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr color default_color{color_magenta};
constexpr milliseconds default_period_time{1250ms};
constexpr int default_period{2 * cube::cube_size_1d};

double_sine_wave::double_sine_wave(engine_context & context) :
    configurable_animation(context),
    update_timer_(context, [this](auto, auto) {
        for (wave & w : waves_)
            w.time_count = (w.time_count + 1) % period_;
        update();
    }),
    fader_(context, {{0.1, 1.0}, 10, 1000ms})
{ }

void double_sine_wave::start()
{
    auto const gradient_start = read_property(color_gradient_start, default_color);
    auto const gradient_end = read_property(color_gradient_end, !default_color);
    period_ = std::max(1, read_property(wave_period, default_period));
    omega_ = (2.0 * M_PI) / period_;

    waves_[0].time_count = 0;
    waves_[0].gradient_start = gradient_start;
    waves_[0].gradient_end = gradient_end;

    waves_[1].time_count = period_ / 3; // Shift 120 degrees
    waves_[1].gradient_start = gradient_end; // Invert color of 2nd wave
    waves_[1].gradient_end = gradient_start;

    update_timer_.start(read_property(wave_period_time_ms, default_period_time) / period_);
    fader_.start();
}

void double_sine_wave::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (wave const & w : waves_) {
        gradient hue({
            {0.0, w.gradient_start},
            {1.0, w.gradient_end},
        });

        for (int i = w.time_count; i < (w.time_count + cube::cube_size_1d); ++i) {
            p.set_color(hue(abs_cos(i * omega_)).vec() * rgb_vec(fader_.value()));

            int z = map(std::sin(i * omega_), unit_circle_range, cube_axis_range);
            int x = i - w.time_count;

            for (int y = 0; y < cube::cube_size_1d; ++y)
                p.draw({x, y, z});
        }
    }
}

void double_sine_wave::stop()
{
    update_timer_.stop();
    fader_.stop();
}

nlohmann::json double_sine_wave::properties_to_json() const
{
    return {
        to_json(wave_period, default_period),
        to_json(wave_period_time_ms, default_period_time),
        to_json(color_gradient_start, default_color),
        to_json(color_gradient_end, default_color),
    };
}

std::vector<double_sine_wave::property_pair_t> double_sine_wave::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, wave_period, default_period),
        from_json(json, wave_period_time_ms, default_period_time),
        from_json(json, color_gradient_start, default_color),
        from_json(json, color_gradient_end, default_color),
    };
}

} // End of namespace
