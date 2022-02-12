#pragma once

#include <cube/core/math.hpp>
#include <cube/specs.hpp>
#include <3rdparty/glm/vec3.hpp>

namespace cube::core
{

using voxel_t = glm::ivec3;

inline voxel_t random_voxel()
{
    constexpr range r{cube_axis_min_value, cube_axis_max_value};
    return {rand(r), rand(r), rand(r)};
}

inline bool visible(voxel_t const & voxel)
{
    constexpr range range{cube_axis_min_value, cube_axis_max_value};
    return within_inclusive(voxel.x, range)
        && within_inclusive(voxel.y, range)
        && within_inclusive(voxel.z, range);
}

} // End of namespace
