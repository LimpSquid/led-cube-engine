#pragma once

#include <gfx/animation_track.h>

namespace cube::gfx::animations
{

class fill_cube : public animation_track
{
public:
    enum : animation_track::property_label_type
    {
        property_cycle_interval_sec     = property_user + 1,
    };

    fill_cube() = default;
    virtual ~fill_cube() override = default;

private:
    virtual void configure(cube::core::animation_config &config) override;
    virtual void time_step() override;
    virtual void paint(cube::core::graphics_device &device) override;

    std::chrono::milliseconds elapsed_ms_;
};

}