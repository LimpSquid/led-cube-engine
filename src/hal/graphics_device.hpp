#pragma once

#ifdef TARGET_MOCK
#include "mock/display.hpp"
#elif TARGET_RPI
#include "rpi/voxel_display.hpp"
#else
#error "Oopsie, unknown target."
#endif
