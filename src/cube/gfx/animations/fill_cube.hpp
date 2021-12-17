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

    fill_cube(core::engine_context & context);

private:
    virtual void configure() override;
    virtual void paint(core::graphics_device & device) override;

    core::tick_subscription::pointer tick_sub_;
};

} // End of namespace
