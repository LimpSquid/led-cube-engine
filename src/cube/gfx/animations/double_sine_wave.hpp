#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/subscriptions.hpp>

namespace cube::gfx::animations
{

class double_sine_wave :
    public configurable_animation
{
public:
    enum : property_label_type
    {
        wave_period = property_custom,      // wave period in number of pixels
        wave_period_time_ms,                // time of one wave period to complete
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

    virtual void configure() override;
    virtual void paint(core::graphics_device & device) override;

    core::tick_subscription::pointer tick_sub_;
    std::array<wave, 2> waves_;
    int period_;
    double omega_;
};

} // End of namespace
