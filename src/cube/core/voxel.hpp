#pragma once

#include <cube/core/math.hpp>
#include <cube/specs.hpp>
#include <glm/vec3.hpp>

namespace cube::core
{

using voxel_t = glm::ivec3;

inline voxel_t random_voxel()
{
    constexpr int mod = cube_size_1d;
    return {std::rand() % mod, std::rand() % mod,  std::rand() % mod};
}

inline bool visible(voxel_t const & voxel)
{
    constexpr Range range = {cube_axis_min_value, cube_axis_max_value};
    return within_range(voxel.x, range)
        && within_range(voxel.y, range)
        && within_range(voxel.z, range);
}

} // End of namespace
