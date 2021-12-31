#pragma once

#include <cube/core/graphics_device.hpp>
#include <cube/core/asio_util.hpp>

namespace hal::mock
{

class window;

class display :
    public cube::core::graphics_device
{
public:
    display(cube::core::engine_context & context);
    virtual ~display() override;

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;

    void schedule_update();
    void update();

    window & window_;
    cube::core::graphics_buffer buffer_;
    cube::core::parent_tracker_t tracker_;
};

} // End of namespace

namespace hal { using graphics_device_t = mock::display; }
