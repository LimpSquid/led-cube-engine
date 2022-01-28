#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class lightning :
    public configurable_animation
{
public:
    lightning(core::engine_context & context);

private:
    PROPERTY_ENUM
    (
        number_of_clouds,       // Number of clouds in the cube
        cloud_radius,           // Radius of the cloud
        cloud_gradient,         // Gradient of the clouds
    )

    struct cloud
    {
        core::voxel_t voxel;
        std::unique_ptr<ease_in_bounce> in_fader;
        std::unique_ptr<ease_out_sine> out_fader;
    };

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;
    virtual nlohmann::json properties_to_json() const override;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    void spawn_cloud(cloud & c);

    core::animation_scene scene_;
    std::vector<cloud> clouds_;
    gradient cloud_gradient_;
    int cloud_radius_;
};

} // End of namespace
