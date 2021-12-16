#pragma once

#include <cube/gfx/animation_track.hpp>
#include <cube/core/voxel.hpp>

namespace cube::gfx::animations
{

class stars :
    public animation_track
{
public:
    enum : property_label_type
    {
        fade_resolution = property_custom,  // number of fade steps
        fade_time_ms,                       // time to complete a single fade cycle for a star
        number_of_stars,
    };

private:
    struct star
    {
        core::voxel_t voxel;
        int fade_step;
    };

    virtual void configure(core::animation_config & config) override;
    virtual void time_step() override;
    virtual void paint(core::graphics_device & device) override;
    star make_unique_star() const;

    std::vector<star> stars_;
    int hue_step_;
    int fade_resolution_;
    double omega_;
};

} // End of namespace
