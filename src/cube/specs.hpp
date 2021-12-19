#pragma once

#include <cassert>

namespace cube
{

constexpr int cube_size_1d = 16;
constexpr int cube_size_2d = cube_size_1d * cube_size_1d;
constexpr int cube_size_3d = cube_size_1d * cube_size_1d * cube_size_1d;
constexpr int cube_axis_min_value = 0;
constexpr int cube_axis_max_value = cube_size_1d - 1;
static_assert((cube_size_1d & 0x01) == 0); // Must be even

} // End of namespace
