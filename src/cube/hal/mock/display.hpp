#pragma once

#include <cube/core/graphics_device.hpp>

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
    virtual void show(core::graphics_buffer const & buffer) override;
    virtual void poll() override;

    window & window_;
    core::graphics_buffer buffer_;
};

} // End of namespace
