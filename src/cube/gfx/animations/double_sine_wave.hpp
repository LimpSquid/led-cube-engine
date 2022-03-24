#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>

namespace cube::gfx::animations
{

class double_sine_wave :
    public configurable_animation
{
public:
    double_sine_wave(core::engine_context & context);

private:
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
        core::color gradient_start;
        core::color gradient_end;
    };

    void start() override;
    void paint(core::graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    core::recurring_timer update_timer_;
    std::array<wave, 2> waves_;
    ease_in_sine fader_;
    int period_;
    double omega_;
};

} // End of namespace
