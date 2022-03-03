#pragma once

#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <cube/core/graphics_device.hpp>

namespace hal::rpi_cube
{

class display :
    public cube::core::graphics_device
{
public:
    display(cube::core::engine_context & context);

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;

    resources resources_;
    bus_comm bus_comm_;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi_cube::display; }
