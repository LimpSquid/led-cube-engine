#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/transition.hpp>

namespace cube::gfx::animations
{

class double_sine_wave :
    public configurable_animation
{
public:
    enum : property_label_type
    {
        wave_period = property_custom,      // number of voxels along the x-axis for one wave period
        wave_period_time_ms,                // time in milliseconds to complete one wave period
        color_gradient_start,               // start color of gradient
        color_gradient_end,                 // end color of gradient
    };

    double_sine_wave(core::engine_context & context);

private:
    struct wave
    {
        int time_count;
        core::color gradient_start;
        core::color gradient_end;
    };

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;

    core::recurring_timer update_timer_;
    std::array<wave, 2> waves_;
    sine_transition fader_;
    int period_;
    double omega_;
};

} // End of namespace
