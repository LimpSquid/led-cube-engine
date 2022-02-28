#pragma once

#include <hal/specs.hpp>
#include <chrono>

namespace cube
{

// global cube specs
constexpr int cube_size_1d{hal::cube_size};
constexpr int cube_size_2d{cube_size_1d * cube_size_1d};
constexpr int cube_size_3d{cube_size_1d * cube_size_1d * cube_size_1d};
constexpr int cube_axis_min_value{0};
constexpr int cube_axis_max_value{cube_size_1d - 1};
static_assert(cube_size_1d > 0); // Must be positive
static_assert((cube_size_1d & 0x01) == 0); // Must be even

// global animation specs
constexpr int animation_scene_fps{hal::animation_scene_fps};
static_assert(animation_scene_fps > 0);
static_assert(animation_scene_fps <= 120); // Limit to some sane number
constexpr auto animation_scene_interval{std::chrono::milliseconds(1000) / animation_scene_fps};


} // End of namespace
