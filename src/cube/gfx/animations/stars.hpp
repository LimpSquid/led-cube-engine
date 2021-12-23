#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class stars :
    public configurable_animation
{
public:
    enum : property_label_type
    {
        fade_time_ms = property_custom, // time in milliseconds to complete one fade cycle
        number_of_stars,                // total number of stars in the cube
    };

    stars(core::engine_context & context);

private:
    struct star
    {
        core::voxel_t voxel;
        int fade_step;
    };

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;
    star make_unique_star() const;

    core::animation_scene scene_;
    std::vector<star> stars_;
    int hue_step_;
    int step_interval_;
    double omega_;
    double omega_hue_;
};

} // End of namespace
