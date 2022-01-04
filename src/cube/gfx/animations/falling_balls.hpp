#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class falling_balls :
    public configurable_animation
{
public:
    PROPERTY_ENUM
    (
        number_of_balls,    // Number of falling balls
        max_ball_size,      // Maximum radius of a ball
        min_ball_size,      // Minimum radius of a ball
    )

    falling_balls(core::engine_context & context);

private:
    struct ball
    {
        int size;
        double mass;
        glm::dvec3 position;
        glm::dvec3 velocity;

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
    int max_size_;
    int min_size_;
};

} // End of namespace
