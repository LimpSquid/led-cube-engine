#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class stars :
    public configurable_animation
{
public:
    PROPERTY_ENUM
    (
        number_of_stars,    // Number of unique stars in the cube
        fade_time_ms        // Fade time of a single star
    )

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
    virtual nlohmann::json properties_to_json() const override;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    star make_star() const;

    core::animation_scene scene_;
    std::vector<star> stars_;
    int hue_step_;
    int step_interval_;
    double omega_;
    double omega_hue_;
};

} // End of namespace
