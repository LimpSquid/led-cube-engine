#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>
#include <cube/core/json_util.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::double_sine_wave> const publisher = {"double_sine_wave"};

constexpr range cube_axis_range = {cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr color default_color = color_magenta;
constexpr milliseconds default_period_time = 1250ms;
constexpr int default_period = 2 * cube::cube_size_1d;

} // End of namespace

namespace cube::gfx::animations
{

double_sine_wave::double_sine_wave(engine_context & context) :
    configurable_animation(context),
    update_timer_(context, [this](auto, auto) {
        for (wave & w : waves_)
            w.time_count = (w.time_count + 1) % period_;
        update();
    }),
    fader_(context, {0.1, 1.0, 10, 1000ms})
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

        for (int i = w.time_count; i < (w.time_count + cube_size_1d); ++i) {
            p.set_color(hue(abs_cos(i * omega_)).vec() * rgb_vec(fader_.value()));

            int z = map(std::sin(i * omega_), unit_circle_range, cube_axis_range);
            int x = i - w.time_count;

            for (int y = 0; y < cube_size_1d; ++y)
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
        make_field(wave_period, read_property(wave_period, default_period)),
        make_field(wave_period_time_ms, read_property(wave_period_time_ms, default_period_time)),
        make_field(color_gradient_start, read_property(color_gradient_start, default_color)),
        make_field(color_gradient_end, read_property(color_gradient_end, !default_color)),
    };
}

std::vector<double_sine_wave::property_pair> double_sine_wave::properties_from_json(nlohmann::json const & json) const
{
    return {
        {wave_period, parse_field(json, wave_period, default_period)},
        {wave_period_time_ms, parse_field(json, wave_period_time_ms, default_period_time)},
        {color_gradient_start, parse_field(json, color_gradient_start, default_color)},
        {color_gradient_end, parse_field(json, color_gradient_end, !default_color)},
    };
}

} // End of namespace
