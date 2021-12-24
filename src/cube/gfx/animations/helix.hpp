#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/transition.hpp>

namespace cube::gfx::animations
{

class helix :
    public configurable_animation
{
public:
    enum : property_label_type
    {
        helix_rotation_time_ms = property_custom,   // time in milliseconds to complete one helix rotation
        helix_phase_shift_cos_factor,               // just play with it and you'll see :-)
        helix_phase_shift_sin_factor,               // just play with it and you'll see :-)
        helix_thickness,                            // thickness of the helix
        helix_length,                               // length of the helix
        color_gradient_start,                       // start color of gradient
        color_gradient_end,                         // end color of gradient
    };

    helix(core::engine_context & context);

private:
    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;

    core::animation_scene scene_;
    gradient hue_;
    sine_transition fader_;
    int step_;
    double omega_;
    double thickness_;
    double length_;
};

} // End of namespace
