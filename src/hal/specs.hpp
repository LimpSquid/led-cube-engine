#pragma once

#ifdef TARGET_MOCK
#include "mock/specs_fwd.hpp"
#elif TARGET_RPI_CUBE
#include "rpi_cube/specs_fwd.hpp"
#else
#error "Oopsie, unknown target."
#endif
