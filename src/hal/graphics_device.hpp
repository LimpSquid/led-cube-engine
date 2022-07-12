#pragma once

#ifdef TARGET_MOCK
    #include "mock/display.hpp"
    #define TARGET_GRAPHICS_DEVICE      mock::display
#elif TARGET_RPI_CUBE
    #include "rpi_cube/display.hpp"
    #define TARGET_GRAPHICS_DEVICE      rpi_cube::display
#else
    #error "Oopsie, unknown target."
#endif

namespace hal
{

using graphics_device_t = TARGET_GRAPHICS_DEVICE;

} // End of namespace
