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

struct particle
{
    double orbit_radius;
    double angle;
    double height;
    double speed_factor;
    double twinkle_phase;
};

struct cosmic_vortex :
    configurable_animation
{
    cosmic_vortex(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    std::vector<particle> particles_;
    std::optional<ease_in_sine> fade_in_;
    std::optional<ease_out_sine> fade_out_;
    gradient gradient_;
    double time_;
};

animation_publisher<cosmic_vortex> const publisher;

constexpr int num_spiral_arms{3};
constexpr double arm_tightness{0.25};
constexpr double disk_thickness_ratio{0.25};
constexpr double core_radius_ratio{0.1};
constexpr unsigned int default_star_count{std::min(2000u, static_cast<unsigned int>(cube::cube_size_2d))};
constexpr milliseconds default_rotation_time{8000ms};
constexpr double default_motion_blur{0.93};
gradient const default_gradient
{
    {0.00, color_white},
    {0.08, lighter(color_yellow, 0.4)},
    {0.20, color_yellow},
    {0.35, color_orange},
    {0.50, color_red},
    {0.65, color_magenta},
    {0.80, color_blue},
    {1.00, color_cyan},
};

cosmic_vortex::cosmic_vortex(engine_context & context) :
    configurable_animation(context)
{ }

void cosmic_vortex::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            gradient_ = read_property<gradient>("gradient");
            auto const star_count = read_property<unsigned int>("star_count");
            double const max_r = cube::cube_size_1d * 0.45;

            particles_.resize(star_count);
            for (auto & p : particles_) {
                p.orbit_radius = randd({1.0, max_r});
                p.angle = randd({0.0, 2.0 * M_PI});
                p.height = randd({-1.0, 1.0}) * max_r * disk_thickness_ratio;
                p.speed_factor = 1.0 / std::pow(p.orbit_radius, 0.6);
                p.twinkle_phase = randd({0.0, 2.0 * M_PI});

                int arm = static_cast<int>(randd({0.0, static_cast<double>(num_spiral_arms - 1)}));
                double arm_angle = 2.0 * M_PI * arm / num_spiral_arms;
                double arm_spread = arm_tightness * (0.5 + randd());
                p.angle += arm_angle + p.orbit_radius * arm_spread;
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

void cosmic_vortex::scene_tick(milliseconds dt)
{
    auto const rotation_time = read_property<milliseconds>("rotation_time_ms");
    double const omega = (2.0 * M_PI) / static_cast<double>(rotation_time.count());
    double const delta = omega * static_cast<double>(dt.count());
    double const max_r = cube::cube_size_1d * 0.45;

    for (auto & p : particles_) {
        p.angle += delta * p.speed_factor;

        p.orbit_radius += std::sin(p.angle * 0.3 + time_ * 0.5) * 0.015;
        p.orbit_radius = std::clamp(p.orbit_radius, 0.5, max_r);

        p.height += std::sin(p.angle * 1.7 + time_ * 0.8) * 0.04;
        p.height = std::clamp(p.height, -max_r * disk_thickness_ratio, max_r * disk_thickness_ratio);
    }
    time_ += static_cast<double>(dt.count()) * 0.001;
}

void cosmic_vortex::paint(graphics_device & device)
{
    double const fade = fade_in_->value() * fade_out_->value();
    double const cube_center = cube::cube_axis_max_value / 2.0;
    double const max_r = cube::cube_size_1d * 0.45;

    double const tilt = std::sin(time_ * 0.06) * 0.35;
    double const ct = std::cos(tilt);
    double const st = std::sin(tilt);

    // Draw core glow
    {
        painter p(device);
        double const core_pulse = 1.0 + 0.12 * std::sin(time_ * 3.0);
        int core_r = std::max(1, static_cast<int>(cube::cube_size_1d * core_radius_ratio * core_pulse));
        auto cc = gradient_(0.0).vec();
        cc *= rgb_vec(fade);
        p.set_color(cc);
        p.sphere(voxel_t{static_cast<int>(cube_center), static_cast<int>(cube_center), static_cast<int>(cube_center)}, core_r);
    }

    // Draw particles in parallel
    auto const half = particles_.size() / 2;
    parallel_for({0, 2}, [&](parallel_range_t range) {
        size_t const start = range.from == 0 ? 0 : half;
        size_t const end = range.from == 0 ? half : particles_.size();

        for (size_t i = start; i < end; ++i) {
            auto const & p = particles_[i];

            double arm_strength = 0.0;
            for (int arm = 0; arm < num_spiral_arms; ++arm) {
                double arm_angle = 2.0 * M_PI * arm / num_spiral_arms + p.orbit_radius * 0.3;
                double diff = std::abs(p.angle - arm_angle);
                diff = std::min(diff, 2.0 * M_PI - diff);
                arm_strength += std::exp(-diff * diff * 6.0);
            }

            double const twinkle = 0.7 + 0.3 * std::sin(p.twinkle_phase + time_ * 2.0);

            double r = p.orbit_radius;
            double a = p.angle;
            double h = p.height;

            double x = r * std::cos(a);
            double y = r * std::sin(a);

            double yt = y * ct - h * st;
            double zt = y * st + h * ct;

            voxel_t voxel = {
                static_cast<int>(std::round(x + cube_center)),
                static_cast<int>(std::round(yt + cube_center)),
                static_cast<int>(std::round(zt + cube_center))
            };

            if (visible(voxel)) {
                double const nrm = std::min(r / max_r, 1.0);
                double const d = arm_strength * twinkle * fade * (0.6 + 0.4 * (1.0 - nrm));
                auto c = gradient_(nrm).vec();
                c *= rgb_vec(d);
                device.draw_with_color(voxel, c);
            }
        }
    }, use_all_cpus);
}

std::unordered_map<std::string, property_value_t> cosmic_vortex::extra_properties() const
{
    return {
        {"star_count", default_star_count},
        {"rotation_time_ms", default_rotation_time},
        {"gradient", default_gradient},
        {"motion_blur", default_motion_blur},
    };
}

} // End of namespace
