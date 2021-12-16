#pragma once

#include <cube/specs.hpp>
#include <glm/vec3.hpp>

namespace cube::core
{

using voxel_t = glm::ivec3;

inline voxel_t random_voxel()
{
    return {
        std::rand() % cube_size_1d,
        std::rand() % cube_size_1d,
        std::rand() % cube_size_1d,
    };
}

} // End of namespace
