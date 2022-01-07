#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/easing.hpp>

namespace cube::gfx::animations
{

class helix :
    public configurable_animation
{
public:
    helix(core::engine_context & context);

private:
    PROPERTY_ENUM
    (
        helix_rotation_time_ms,         // Time in milliseconds to complete one helix rotation
        helix_phase_shift_cos_factor,   // Just play with it and you'll see :-)
        helix_phase_shift_sin_factor,   // Same...
        helix_thickness,                // Cross-section radius of the helix
        helix_length,                   // Length of the helix
        color_gradient_start,           // Start color of the helix gradient
        color_gradient_end              // End color of the helix gradient
    )

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;
    virtual nlohmann::json properties_to_json() const override;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    core::animation_scene scene_;
    gradient hue_;
    ease_in_sine fader_;
    int step_;
    int thickness_;
    double omega_;
    double length_;
};

} // End of namespace
