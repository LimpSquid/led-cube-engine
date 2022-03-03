#include <hal/rpi_cube/bus_comm.hpp>

namespace hal::rpi_cube
{

bus_comm::bus_comm(iodev & device) :
    device_(device),
    read_subscription_(device_.subscribe([this]() { do_read(); }))
{

}

void bus_comm::do_read()
{
    // Todo: remove, just for compile tests
    struct packet
    {
        int a, b, c;
    };

    using multi_packet = std::array<packet, 2>;

    if (device_.is_readable<multi_packet>()) {
        multi_packet p;
        device_.read_into(p);
    }
}

} // End of namespace
