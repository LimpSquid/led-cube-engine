#pragma once

#include <cube/core/graphics_device.h>

namespace cube::hal::opengl
{

class window;

class mock_display :
    public core::graphics_device
{
public:
    mock_display();
    virtual ~mock_display() override;

private:
    virtual void show(core::graphics_buffer & buffer) override;
    virtual void poll() override;

    window & window_;
};

}
