#pragma once

#include <hal/rpi/peripherals.hpp>
#include <cube/core/graphics_device.hpp>

namespace hal::rpi
{

class display :
    public cube::core::graphics_device
{
public:
    display(cube::core::engine_context & context);

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;

    peripherals peripherals_;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi::display; }
