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
    // PROPERTY_ENUM
    // (
    // )

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
    virtual std::vector<property_pair> properties_from_json(nlohmann::json const & json) const override;

    void spawn_cloud(cloud & c);

    core::animation_scene scene_;
    std::vector<cloud> clouds_;
};

} // End of namespace
