#pragma once

#include <cube/core/graphics_device.hpp>

namespace hal::mock
{

class window;

class display :
    public cube::core::graphics_device
{
public:
    display();
    virtual ~display() override;

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;
    virtual void poll() override;

    window & window_;
    cube::core::graphics_buffer buffer_;
};

} // End of namespace

namespace hal { using graphics_device_t = mock::display; }
