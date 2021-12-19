#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/transition.hpp>

namespace cube::gfx::animations
{

class helix :
    public configurable_animation
{
public:
    enum : property_label_type
    {

    };

    helix(core::engine_context & context);

private:
    struct wave
    {
        int time_count;
        core::color gradient_start;
        core::color gradient_end;
    };

    virtual void configure() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;

    core::tick_subscription::pointer tick_sub_;
    int period_;
    double omega_;
};

} // End of namespace
