#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>
#include <cube/core/parallel.hpp>
#include <cube/core/graphics_device.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct curtain
{
    double x;
    double drift;
    double speed;
    double hue_shift;
    double intensity;
};

struct aurora :
    configurable_animation
{
    aurora(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    std::vector<curtain> curtains_;
    std::optional<ease_in_sine> fade_in_;
    std::optional<ease_out_sine> fade_out_;
    gradient gradient_;
    double time_;
};

animation_publisher<aurora> const publisher;

constexpr int default_curtain_count{5};
constexpr double curtain_spacing{0.22};
constexpr double default_motion_blur{0.92};
gradient const default_gradient
{
    {0.00, darker(color_green, 0.3)},
    {0.20, color_green},
    {0.40, lighter(color_cyan, 0.3)},
    {0.55, color_cyan},
    {0.70, color_magenta},
    {0.85, lighter(color_pink, 0.3)},
    {1.00, color_white},
};

aurora::aurora(engine_context & context) :
    configurable_animation(context)
{ }

void aurora::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            gradient_ = read_property<gradient>("gradient");
            auto const curtain_count = read_property<int>("curtain_count");

            curtains_.resize(static_cast<std::size_t>(curtain_count));
            double const spacing = cube::cube_size_1d * curtain_spacing;
            double const offset = (cube::cube_size_1d - spacing * (curtain_count - 1)) * 0.5;
            for (int i = 0; i < curtain_count; ++i) {
                auto & c = curtains_[static_cast<std::size_t>(i)];
                c.x = offset + i * spacing + randd({-0.15, 0.15}) * spacing;
                c.drift = randd({-0.02, 0.02});
                c.speed = randd({0.8, 1.4});
                c.hue_shift = randd({0.0, 2.0 * M_PI});
                c.intensity = randd({0.6, 1.0});
            }

            time_ = randd({0.0, 100.0});

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

void aurora::scene_tick(milliseconds dt)
{
    double const delta = static_cast<double>(dt.count()) * 0.001;
    time_ += delta;

    for (auto & c : curtains_)
        c.x += c.drift * delta;
}

void aurora::paint(graphics_device & device)
{
    double const fade = fade_in_->value() * fade_out_->value();
    double const speed = read_property<double>("speed");

    auto const cw = static_cast<double>(cube::cube_size_1d) * 0.12;
    double const cw_inv = 1.0 / cw;

    parallel_for({0, cube::cube_size_1d}, [&](parallel_range_t x_range) {
        for (int x = x_range.from; x < x_range.to; ++x) {
            for (int y = 0; y < cube::cube_size_1d; ++y) {
                double const yn = static_cast<double>(y) / cube::cube_axis_max_value;
                double const y_fade = std::sin(yn * M_PI);

                for (int z = 0; z < cube::cube_size_1d; ++z) {
                    double total = 0.0;
                    double hue_offset = 0.0;

                    for (auto const & c : curtains_) {
                        double dx = static_cast<double>(x) - c.x;
                        double dist = std::abs(dx) * cw_inv;
                        if (dist >= 1.0) continue;

                        double const falloff = 1.0 - dist * dist;

                        double const t = time_ * speed * c.speed;
                        double const yn2 = yn * 8.0 + 2.0;
                        double const zn2 = static_cast<double>(z) * 0.08 + 0.5;

                        double ripple = std::sin(yn2 * 1.8 + zn2 * 2.5 + t * 1.3)
                            + std::sin(yn2 * 3.2 - zn2 * 1.7 + t * 0.7) * 0.55
                            + std::sin(yn2 * 0.9 + zn2 * 3.1 + t * 2.1 + 1.2) * 0.35
                            + std::sin((yn2 + zn2) * 1.4 + t * 0.4) * 0.25
                            + std::sin(std::sqrt(yn2 * zn2) * 2.0 + t * 1.1) * 0.2;

                        double intensity = (ripple * 0.5 + 0.5) * falloff * c.intensity;

                        if (intensity > 0.02) {
                            total += intensity;
                            hue_offset += intensity * c.hue_shift;
                        }
                    }

                    if (total > 0.02) {
                        total = std::min(total, 1.0);
                        double const h = std::fmod(hue_offset / total, 2.0 * M_PI) / (2.0 * M_PI);
                        double const gp = std::fmod(yn + h * 0.15 + time_ * 0.02, 1.0);
                        auto c = gradient_(gp).vec();

                        double const brightness = total * fade * (0.5 + 0.5 * y_fade);
                        c *= rgba_vec(brightness);

                        device.draw_with_color({x, y, z}, c);
                    }
                }
            }
        }
    }, use_all_cpus);
}

std::unordered_map<std::string, property_value_t> aurora::extra_properties() const
{
    return {
        {"curtain_count", default_curtain_count},
        {"speed", 1.0},
        {"gradient", default_gradient},
        {"motion_blur", default_motion_blur},
    };
}

} // End of namespace
