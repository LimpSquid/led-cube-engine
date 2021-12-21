#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/timers.hpp>

namespace cube::gfx::animations
{

class fill_cube :
    public configurable_animation
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
    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;

    core::recurring_timer update_timer_;
};

} // End of namespace
