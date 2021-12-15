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
        frame_rate_ms = property_custom, // Todo: possibly move this ot animation_track as this will likely be used a lot
        period_wave_1,
        period_wave_2,
        color_wave_1,
        color_wave_2,
    };

private:
    struct wave
    {
        int time_count;
        int period;
        double omega;
        core::color color;
    };

    virtual void configure(core::animation_config & config) override;
    virtual void time_step() override;
    virtual void paint(core::graphics_device & device) override;
    void init_waves();

    std::array<wave, 2> waves;
};

} // end of namespace
