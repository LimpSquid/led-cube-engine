#pragma once

#ifdef TARGET_MOCK
#include "mock/display.hpp"
#elif TARGET_RPI_CUBE
#include "rpi_cube/display.hpp"
#else
#error "Oopsie, unknown target."
#endif
