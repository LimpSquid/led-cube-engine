#pragma once

#include <cube/core/graphics_device.h>

namespace cube::hal::mock
{

class window;

class display :
    public core::graphics_device
{
public:
    display();
    virtual ~display() override;

private:
    virtual void show(core::graphics_buffer & buffer) override;
    virtual void poll() override;

    window & window_;
};

}
