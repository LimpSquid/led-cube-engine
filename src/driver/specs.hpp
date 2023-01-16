#pragma once

#ifdef TARGET_MOCK
    #define TARGET_CUBE_SIZE            32
    #define TARGET_ANIMATION_SCENE_FPS  60
#elif TARGET_RPI_CUBE
    #define TARGET_CUBE_SIZE            16
    #define TARGET_ANIMATION_SCENE_FPS  60
#else
    #error "Oopsie, unknown target."
#endif

namespace driver
{

constexpr int cube_size{TARGET_CUBE_SIZE};
constexpr int animation_scene_fps{TARGET_ANIMATION_SCENE_FPS};

} // End of namespace
