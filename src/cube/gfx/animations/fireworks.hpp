#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/voxel.hpp>

namespace cube::core { class painter; }
namespace cube::gfx::animations
{

class fireworks :
    public configurable_animation
{
public:
    fireworks(core::engine_context & context);

private:
    PROPERTY_ENUM
    (
        number_of_rockets,      // Number of rockets
        number_of_fragments,    // Number of fragments when exploded
        explosion_force,        // Factor to limit or increase the explosion force
        rocket_trail_radius,    // Radius of the rocket trail
        rocket_colors,          // Array of rocket colors to pick from
    )

    struct particle
    {
        int radius;
        glm::dvec3 position;
        glm::dvec3 velocity;
        gradient hue;

        void move(std::chrono::milliseconds const & dt);
        void paint(cube::core::painter & p) const;
    };

    struct rocket
    {
        enum state
        {
            flying,
            exploded,
            completed,
        };

        particle trail;
        std::vector<particle> fragments;
        double explosion_force;
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

    rocket make_rocket() const;

    core::animation_scene scene_;
    std::vector<rocket> rockets_;
    std::vector<core::color> rocket_colors_;
    double explosion_force_;
    unsigned int num_fragments_;
    int trail_radius_;
};

} // End of namespace
