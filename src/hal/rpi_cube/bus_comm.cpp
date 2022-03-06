#include <hal/rpi_cube/bus_comm.hpp>
#include <cassert>

using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr milliseconds transfer_timeout{50};

void throw_if_ne(hal::rpi_cube::bus_comm::bus_state expected, hal::rpi_cube::bus_comm::bus_state actual)
{
    if (expected != actual)
        std::runtime_error("Expected bus state: " + std::to_string(expected) + ", got: " + std::to_string(actual));
}


} // End of namespace

namespace hal::rpi_cube
{

void_or_error verify_response_code(bus_response_code response)
{
    switch (response) {
        case bus_response_code::ok:                     return {};
        case bus_response_code::err_unknown:            return unexpected_error{"Unknown error"};
        case bus_response_code::err_again:              return unexpected_error{"Try again"};
        case bus_response_code::err_invalid_payload:    return unexpected_error{"Invalid payload"};
        case bus_response_code::err_invalid_command:    return unexpected_error{"Invalid command"};
        default:;
    }

     return unexpected_error{"???"};
}

bus_comm::bus_comm(iodev & device) :
    device_(device),
    read_subscription_(device_.subscribe([this]() { do_read(); })),
    transfer_watchdog_(device.context(), [this](auto, auto) { do_timeout(); }),
    state_(idle)
{

}

void bus_comm::do_read()
{
    // Unexpected data from device, bus error
    if (state_ != transfer)
        return switch_state(error);

    if (!device_.is_readable<bus_frame>())
        return;

    // Transfer done, stop watchdog
    transfer_watchdog_.stop();

    // Read frame and forward to handler
    auto frame = read_frame();
    auto const & job = jobs_.back();
    job.handler(std::move(frame)); // Todo: maybe retry?

    jobs_.pop_back();
    if (jobs_.empty())
        switch_state(idle);
    else
        do_write_one();
}

void bus_comm::do_write()
{
    throw_if_ne(idle, state_); // Should never happen
    switch_state(transfer);
    do_write_one();
}

void bus_comm::do_write_one()
{
    throw_if_ne(transfer, state_); // Should never happen

    if (jobs_.empty())
        throw std::runtime_error("No job to write");

    auto const & job = jobs_.back();

    // Unable to write, bus error
    if (device_.is_writeable<bus_frame>()) {
        device_.write_from(job.frame);
        transfer_watchdog_.start(transfer_timeout);
    } else
        switch_state(error);
}

void bus_comm::do_timeout()
{
    switch_state(error);
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

expected_or_error<bus_comm::bus_frame> bus_comm::read_frame()
{
    if (!device_.is_readable<bus_frame>())
        return unexpected_error{"Unable to read frame from device"};

    bus_frame frame;
    device_.read_into(frame);

    if (false) // Todo: check crc
        return unexpected_error{"CRC error"};
    return {std::move(frame)};
}

void bus_comm::add_job(job && j)
{
    jobs_.push_front(std::move(j));

    if (jobs_.size() == 1)
        do_write();
}

} // End of namespace
