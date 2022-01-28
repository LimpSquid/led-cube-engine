#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class falling_balls :
    public configurable_animation
{
public:
    falling_balls(core::engine_context & context);

private:
    PROPERTY_ENUM
    (
        number_of_balls,    // Number of falling balls
        max_ball_radius,    // Maximum radius of a ball
        min_ball_radius,    // Minimum radius of a ball
        ball_colors,        // Array of ball colors to pick from
    )

    struct ball
    {
        int radius;
        double mass;
        glm::dvec3 position;
        glm::dvec3 velocity;
        core::color color;

        void move(std::chrono::milliseconds const & dt);
    };

    virtual void start() override;
    virtual void paint(core::graphics_device & device) override;
    virtual void stop() override;
    virtual nlohmann::json properties_to_json() const override;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    ball make_ball() const;

    core::animation_scene scene_;
    std::vector<ball> balls_;
    std::vector<core::color> ball_colors_;
    int max_radius_;
    int min_radius_;
};

} // End of namespace
