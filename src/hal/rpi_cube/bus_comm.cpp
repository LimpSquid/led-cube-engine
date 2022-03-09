#include <hal/rpi_cube/bus_comm.hpp>
#include <cassert>

using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

constexpr milliseconds transfer_timeout{50};
constexpr unsigned max_attempts{3};

void throw_if_ne(hal::rpi_cube::bus_state expected, hal::rpi_cube::bus_state actual)
{
    if (expected != actual)
        std::runtime_error("Expected bus state: "s + to_string(expected) + ", got: " + to_string(actual));
}

} // End of namespace

namespace hal::rpi_cube
{

bus_comm::bus_comm(iodev & device) :
    device_(device),
    read_subscription_(device_.subscribe([this]() { do_read(); })),
    transfer_watchdog_(device.context(), [this](auto, auto) { do_timeout(); }),
    state_(bus_state::idle)
{ }

void bus_comm::do_read()
{
    // Unexpected data from device, bus error
    if (state_ != bus_state::transfer)
        return switch_state(bus_state::error);

    if (!device_.is_readable<raw_frame>())
        return;

    // Transfer done, stop watchdog
    transfer_watchdog_.stop();

    // Read frame and forward to handler
    auto frame = read_frame();
    auto & job = jobs_.back();

    std::visit([&](auto & params) {
        if constexpr (std::is_same_v<decltype(params), unicast_params>) {
            if (!frame && !frame.error().response_code && params.attempt < max_attempts) {
                params.attempt++;
                jobs_.push_front(std::move(job));
            } else if (params.handler)
                params.handler(std::move(frame));
            do_finish();
        } else
            throw std::runtime_error("Unexpected read for current job");
    }, job.params);
}

void bus_comm::do_write()
{
    throw_if_ne(bus_state::idle, state_); // Should never happen
    switch_state(bus_state::transfer);
    do_write_one();
}

void bus_comm::do_write_one()
{
    throw_if_ne(bus_state::transfer, state_); // Should never happen

    if (jobs_.empty())
        throw std::runtime_error("No job to write");

    // Unable to write, bus error
    if (device_.is_writeable<raw_frame>()) {
        auto & job = jobs_.back();
        device_.write_from(job.frame);

        std::visit([&](auto & params) {
            if constexpr (std::is_same_v<decltype(params), unicast_params>) {
                params.attempt++;
                transfer_watchdog_.start(transfer_timeout);
            } else if constexpr (std::is_same_v<decltype(params), broadcast_params>) {
                if (params.handler)
                    params.handler();
                do_finish();
            }
        }, job.params);
    } else
        switch_state(bus_state::error);
}

void bus_comm::do_timeout()
{
    throw_if_ne(bus_state::transfer, state_); // Should never happen

    auto & job = jobs_.back();

    std::visit([&](auto & params) {
        if constexpr (std::is_same_v<decltype(params), unicast_params>) {
            if (params.attempt < max_attempts)
                jobs_.push_front(std::move(job));
            else if (params.handler)
                params.handler(error{"Timeout", {}});
            do_finish();
        } else
            throw std::runtime_error("Unexpected timeout for current job");
    }, job.params);
}

void bus_comm::do_finish()
{
    throw_if_ne(bus_state::transfer, state_); // Should never happen

    jobs_.pop_back();
    if (jobs_.empty())
        switch_state(bus_state::idle);
    else
        do_write_one();
}

void bus_comm::switch_state(bus_state state)
{
    if (state_ == state)
        throw std::runtime_error("Already switched to state: "s + to_string(state));

    // Todo: error handling
    switch (state) {
        default:;
    }
    state_ = state;
}

bus_comm::frame_or_error bus_comm::read_frame()
{
    if (!device_.is_readable<raw_frame>())
        return error{"Unable to read frame from device", {}};

    raw_frame frame;
    device_.read_into(frame);

    if (crc16_generator{}(&frame, sizeof(frame))) // If the CRC yields non-zero, then the frame is garbled
        return error{"CRC error", {}};

    if (frame.request)
        return error{"Received request message while expecting a response", {}};

    auto response_code = static_cast<bus_response_code>(frame.command_or_response);
    auto make_error = [response_code](char const * const what) { return error{what, response_code}; };
    switch (response_code) {
        case bus_response_code::err_unknown:            return make_error("Unknown error");
        case bus_response_code::err_again:              return make_error("Try again");
        case bus_response_code::err_invalid_payload:    return make_error("Invalid payload");
        case bus_response_code::err_invalid_command:    return make_error("Invalid command");
        default:;
    }

    return {std::move(frame)};
}

void bus_comm::add_job(job && j, bool high_prio)
{
    if (high_prio && !jobs_.empty()) // A job is being handled
        jobs_.insert(jobs_.end() - 1, std::move(j));
    else
        jobs_.push_front(std::move(j));

    if (jobs_.size() == 1)
        do_write();
}

} // End of namespace
