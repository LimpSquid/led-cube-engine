#pragma once

#include <cube/core/graphics_device.h>

namespace cube::hal
{

class opengl_display :
    public core::graphics_device
{
public:
    opengl_display();
    virtual ~opengl_display() override;

private:
    virtual void show(core::graphics_buffer & buffer) override;
    virtual void poll() override;
};

}
