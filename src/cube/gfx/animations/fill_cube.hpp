#pragma once

#include <cube/gfx/animation_track.hpp>

namespace cube::gfx::animations
{

class fill_cube :
    public animation_track
{
public:
    enum : property_label_type
    {
        cycle_interval_sec  = property_custom,
        disable_red,
        disable_green,
        disable_blue,
    };

private:
    virtual void configure(core::animation_config &config) override;
    virtual void time_step() override;
    virtual void paint(core::graphics_device &device) override;
};

}
