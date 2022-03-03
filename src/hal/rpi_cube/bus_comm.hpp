#pragma once

#include <hal/rpi_cube/iodev.hpp>

namespace hal::rpi_cube
{

class bus_comm
{
public:
    bus_comm(iodev & device);

private:
    iodev & device_;
    iodev_subscription read_subscription_;

    void do_read();
};

} // End of namespace
