#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/noise.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/parallel.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct noise_v1 :
    configurable_animation
{
    noise_v1(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    std::optional<ease_in_sine> fade_in_;
    std::optional<ease_out_sine> fade_out_;
    gradient gradient_;
    float time_;
};

animation_publisher<noise_v1> const publisher;

constexpr range grow_shrink_range{0.04, 0.08}; // TODO: scale it according to cube size
constexpr double default_time_rate{0.00025};
gradient const default_gradient
{
    {0.0, color_magenta},
    {0.4, color_transparent},
    {0.5, color_transparent},
    {0.6, color_transparent},
    {1.0, color_cyan},
};

noise_v1::noise_v1(engine_context & context) :
    configurable_animation(context)
{
    noise_reseed();
}

void noise_v1::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running:
            gradient_ = read_property<gradient>("gradient");
            time_ = randf({0, 100.0f});

            fade_in_.emplace(context(), easing_config{{0.0, 1.0}, 50, get_transition_time()});
            fade_out_.emplace(context(), easing_config{{1.0, 0.0}, 50, get_transition_time()});

            fade_in_->start();
            break;
        case animation_state::stopping:
            fade_out_->start();
            break;
        case animation_state::stopped:
            fade_in_->stop();
            fade_out_->stop();
            break;
    }
}

void noise_v1::scene_tick(milliseconds dt)
{
    auto const rate = read_property<double>("time_rate");

    time_ += static_cast<float>(static_cast<unsigned int>(dt.count()) * rate);
}

void noise_v1::paint(graphics_device & device)
{
    painter p(device);

    auto const grow_shrink_rate = time_ * 0.5;
    auto const grow_shrink_scalar = static_cast<float>(map(std::sin(grow_shrink_rate), unit_circle_range, grow_shrink_range));

    parallel_for({0, cube::cube_size_1d}, [&](parallel_range_t range) {
        for (int x = range.from; x < range.to; ++x) {
            auto const xx = static_cast<float>(x) * grow_shrink_scalar;
            for (int y = 0; y < cube::cube_size_1d; ++y) {
                auto const yy = static_cast<float>(y) * grow_shrink_scalar;
                for (int z = 0; z < cube::cube_size_1d; ++z) {
                    auto const zz = static_cast<float>(z) * grow_shrink_scalar;
                    auto const gp = map(noise({xx, yy, zz, time_}), noise_range, gradient_pos_range);

                    auto c = gradient_(gp).vec();
                    c *= rgb_vec(fade_in_->value());
                    c *= rgb_vec(fade_out_->value());

                    device.draw_with_color({x, y, z}, c); // Draw directly to device since painter is not thread safe
                }
            }
        }
    }, use_all_cpus);
}

std::unordered_map<std::string, property_value_t> noise_v1::extra_properties() const
{
    return {
        {"gradient", default_gradient},
        {"time_rate", default_time_rate},
    };
}

} // End of namespace
