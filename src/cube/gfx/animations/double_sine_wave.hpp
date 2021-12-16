#pragma once

#include <cube/gfx/animation_track.hpp>

namespace cube::gfx::animations
{

class double_sine_wave :
    public animation_track
{
public:
    enum : property_label_type
    {
        wave_period = property_custom,      // wave period in number of pixels
        wave_period_time_ms,                // time of one wave period to complete
        color_gradient_start,               // start color of gradient
        color_gradient_end,                 // end color of gradient
    };

private:
    struct wave
    {
        int time_count;
        core::color gradient_start;
        core::color gradient_end;
    };

    virtual void configure(core::animation_config & config) override;
    virtual void time_step() override;
    virtual void paint(core::graphics_device & device) override;

    std::array<wave, 2> waves_;
    int period_;
    double omega_;
};

} // End of namespace
