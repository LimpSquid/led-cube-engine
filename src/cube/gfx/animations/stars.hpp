#pragma once

#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/timers.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class stars :
    public configurable_animation
{
public:
    enum : property_label_type
    {
        fade_resolution = property_custom,  // number of fade steps of one star fade cycle
        fade_time_ms,                       // time in milliseconds to complete one fade cycle
        number_of_stars,
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

    core::recurring_timer update_timer_;
    std::vector<star> stars_;
    int hue_step_;
    int resolution_;
    double omega_;
};

} // End of namespace
