#include <hal/rpi_cube/bus_comm.hpp>
#include <cassert>

using namespace cube::core;

namespace
{

void throw_if_ne(hal::rpi_cube::bus_comm::bus_state expected, hal::rpi_cube::bus_comm::bus_state actual)
{
    if (expected != actual)
        std::runtime_error("Expected bus state: " + std::to_string(expected) + ", got: " + std::to_string(actual));
}

} // End of namespace

namespace hal::rpi_cube
{

bus_comm::bus_comm(iodev & device) :
    device_(device),
    read_subscription_(device_.subscribe([this]() { do_read(); })),
    timeout_timer_(device.context(), [this](auto, auto) { do_timeout(); }),
    state_(idle)
{

}

void bus_comm::do_read()
{
    // Unexpected data from device, bus error
    if (state_ != transfer)
        return switch_state(error);

    // Todo read stuf

    jobs_.pop_back();
    if (jobs_.empty())
        switch_state(idle);
    else
        do_write_one();
}

void bus_comm::do_write()
{
    assert(!jobs_.empty());
    throw_if_ne(idle, state_); // Should never happen
    switch_state(transfer);
    do_write_one();
}

void bus_comm::do_write_one()
{
    throw_if_ne(transfer, state_); // Should never happen

    // Unable to write, bus error
    if (device_.is_writeable<int>()) { // Todo use actual data type
        int dummy{};
        device_.write_from(dummy);
        // Todo: start timeout timer
    } else
        switch_state(error);
}

void bus_comm::do_timeout()
{

}

void bus_comm::switch_state(bus_state state)
{
    if (state_ == state)
        throw std::runtime_error("Already switched to state: " + std::to_string(state));

    switch (state) {
        default:;
    }
    state_ = state;
}

} // End of namespace
