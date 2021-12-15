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
        wave_period = property_custom,
        wave_period_time_ms,
        color_gradient_start,
        color_gradient_end,
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
    void init_waves();

    std::array<wave, 2> waves_;
    int period_;
    double omega_;
};

} // End of namespace
