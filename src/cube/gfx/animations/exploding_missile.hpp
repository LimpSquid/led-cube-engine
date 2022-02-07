#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/voxel.hpp>

namespace cube::core { class painter; }
namespace cube::gfx::animations
{

class exploding_missile :
    public configurable_animation
{
public:
    exploding_missile(core::engine_context & context);

private:
    // PROPERTY_ENUM
    // (
    // )

    struct particle
    {
        glm::dvec3 position;
        glm::dvec3 velocity;
        gradient hue;

        void move(std::chrono::milliseconds const & dt);
        void paint(cube::core::painter & p) const;
    };

    struct missile
    {
        enum state
        {
            flying,
            exploded,
            completed,
        };

        particle trail;
        std::vector<particle> fragments;
        state state{flying};

        void update(std::chrono::milliseconds const & dt);
        void paint(cube::core::painter & p) const;
        void explode();
    };

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;
    virtual nlohmann::json properties_to_json() const override;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    missile make_missile() const;

    core::animation_scene scene_;
    std::vector<missile> missiles_;
};

} // End of namespace
