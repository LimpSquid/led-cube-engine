#pragma once

#include <cube/core/graphics_device.hpp>
#include <cube/core/events.hpp>

namespace hal::mock
{

class window;

class display :
    public cube::core::graphics_device
{
public:
    display(cube::core::engine_context & context);

private:
    void show(cube::core::graphics_buffer const & buffer) override;

    void update();

    window & window_;
    cube::core::graphics_buffer buffer_;
    cube::core::function_invoker invoker_;
};

} // End of namespace

namespace hal { using graphics_device_t = mock::display; }
