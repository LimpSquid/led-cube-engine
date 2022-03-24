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
        number_of_shells,       // Number of shells
        number_of_fragments,    // Number of fragments when exploded
        explosion_force,        // Factor to limit or increase the explosion force
        shell_radius,           // Radius of the shell
        shell_colors,           // Array of shell colors to pick from
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

    struct shell
    {
        enum state
        {
            flying,
            exploded,
            completed,
        };

        particle shell;
        std::vector<particle> fragments;
        double explosion_force;
        state state{flying};

        void update(std::chrono::milliseconds const & dt);
        void paint(cube::core::painter & p) const;
        void explode();
    };

    void start() override;
    void paint(core::graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    shell make_shell() const;

    core::animation_scene scene_;
    std::vector<shell> shells_;
    std::vector<core::color> shell_colors_;
    double explosion_force_;
    unsigned int num_fragments_;
    int shell_radius_;
};

} // End of namespace
