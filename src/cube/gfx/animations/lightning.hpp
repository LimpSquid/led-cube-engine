#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class lightning :
    public configurable_animation
{
public:
    PROPERTY_ENUM
    (
        number_of_clouds,   // Number of clouds in the cube
        cloud_size,         // Scatter size of the cloud
        cloud_color,        // Color of the cloud
    )

    lightning(core::engine_context & context);

private:
    struct cloud
    {
        core::voxel_t voxel;
        std::unique_ptr<ease_in_bounce> fader;
    };

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;
    virtual nlohmann::json properties_to_json() const override;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    void spawn_cloud(cloud & c);

    core::animation_scene scene_;
    std::vector<cloud> clouds_;
    gradient hue_;
    double cloud_size_;
};

} // End of namespace
