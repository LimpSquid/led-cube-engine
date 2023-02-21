#include <driver/rpi_cube/bus_comm.hpp>
#include <cube/core/logging.hpp>
#include <cassert>

using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

constexpr unsigned int max_attempts{3};
constexpr milliseconds reset_time{500};

void throw_if_ne(driver::rpi_cube::bus_state expected, driver::rpi_cube::bus_state actual)
{
    if (expected != actual)
        std::runtime_error("Expected bus state: "s + to_string(expected) + ", got: " + to_string(actual));
}

} // End of namespace

namespace driver::rpi_cube
{

bus_comm::bus_comm(iodev & device) :
    device_(device),
    subscriptions_
    {
        device_.subscribe(iodev::ready_read, std::bind(&bus_comm::do_read, this)),
        device_.subscribe(iodev::transfer_complete, std::bind(&bus_comm::do_transfer_complete, this))
    },
    response_watchdog_(device.context(), std::bind(&bus_comm::do_timeout, this)),
    reset_timer_(device.context(), std::bind(&bus_comm::do_reset, this)),
    state_(bus_state::idle),
    job_id_(0)
{ }

bus_state bus_comm::state() const
{
    return state_;
}

void bus_comm::do_read()
{
    if (state_ == bus_state::error)
        return;

    // Unexpected data from device, bus error
    if (state_ != bus_state::transfer)
        return switch_state(bus_state::error);

    if (!device_.is_readable<raw_frame>())
        return;

    // Transfer done, stop watchdog
    response_watchdog_.stop();

    // Read frame and forward to handler
    auto frame = read_frame();
    auto & job = jobs_.back();

    bool const bus_error = !frame && !frame.error().response_code;
    if (bus_error)
        device_.clear(iodev::input);

    std::visit([&](auto & params) {
        using params_t = std::remove_reference_t<decltype(params)>;

        if constexpr (std::is_same_v<params_t, unicast_params>) {
            if (bus_error && params.attempt < max_attempts) {
                LOG_WRN("Bus error",
                    LOG_ARG("job_id", job.id),
                    LOG_ARG("error", frame.error().what),
                    LOG_ARG("address", as_hex(job.frame.address)),
                    LOG_ARG("command", as_hex(job.frame.command_or_response)),
                    LOG_ARG("attempt", params.attempt));
                jobs_.push_front(std::move(job));
            } else if (params.handler) {
                if (!frame)
                    LOG_DBG("Bus error response",
                        LOG_ARG("job_id", job.id),
                        LOG_ARG("error", frame.error().what),
                        LOG_ARG("address", as_hex(job.frame.address)),
                        LOG_ARG("command", as_hex(job.frame.command_or_response)));
                params.handler(std::move(frame));
            }
            do_finish();
        } else
            throw std::runtime_error("Unexpected read for current job");
    }, job.params);
}

void bus_comm::do_transfer_complete()
{
    if (state_ == bus_state::error)
        return;

    // Unexpected transfer completion from device, bus error
    if (state_ != bus_state::transfer)
        return switch_state(bus_state::error);

    auto & job = jobs_.back();
    std::visit([&](auto & params) {
        using params_t = std::remove_reference_t<decltype(params)>;

        if constexpr (std::is_same_v<params_t, broadcast_params>) {
            if (params.handler)
                params.handler({});
            do_finish();
        }
    }, job.params);
}

void bus_comm::do_write()
{
    if (state_ == bus_state::error)
        return;

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
            using params_t = std::remove_reference_t<decltype(params)>;

            if constexpr (std::is_same_v<params_t, unicast_params>) {
                params.attempt++;
                response_watchdog_.start(params.response_timeout);
            } else if constexpr (!std::is_same_v<params_t, broadcast_params>)
                throw std::runtime_error("Unexpected write for current job");
        }, job.params);
    } else
        switch_state(bus_state::error);
}

void bus_comm::do_timeout()
{
    if (state_ == bus_state::error)
        return;

    throw_if_ne(bus_state::transfer, state_); // Should never happen

    auto & job = jobs_.back();

    std::visit([&](auto & params) {
        using params_t = std::remove_reference_t<decltype(params)>;

        if constexpr (std::is_same_v<params_t, unicast_params>) {
            LOG_DBG("Bus timeout",
                LOG_ARG("job_id", job.id),
                LOG_ARG("address", as_hex(job.frame.address)),
                LOG_ARG("command", as_hex(job.frame.command_or_response)),
                LOG_ARG("attempt", params.attempt));
            if (params.attempt < max_attempts)
                jobs_.push_front(std::move(job));
            else if (params.handler)
                params.handler(bus_error{"Timeout", {}});
            do_finish();
        } else
            throw std::runtime_error("Unexpected timeout for current job");
    }, job.params);
}

void bus_comm::do_finish()
{
    if (state_ == bus_state::error)
        return;

    throw_if_ne(bus_state::transfer, state_); // Should never happen

    jobs_.pop_back();
    if (jobs_.empty())
        switch_state(bus_state::idle);
    else
        do_write_one();
}

void bus_comm::do_reset()
{
    throw_if_ne(bus_state::error, state_); // Should never happen

    device_.clear(iodev::all_directions);
    switch_state(bus_state::idle);

    if (!jobs_.empty())
        do_write();
}

void bus_comm::switch_state(bus_state state)
{
    if (state_ == state)
        throw std::runtime_error("Already switched to state: "s + to_string(state));

    switch (state) {
        case bus_state::error: {
            LOG_ERR("Bus error",
                LOG_ARG("from", to_string(state_)),
                LOG_ARG("to", to_string(state)));

            if (!jobs_.empty()) {
                auto & job = jobs_.back();
                std::visit([&](auto & params) {
                    if (params.handler)
                        params.handler(bus_error{"Bus error", {}});
                }, job.params);
                jobs_.pop_back();
            }

            response_watchdog_.stop();
            reset_timer_.start(reset_time);
            break;
        }
        default:;
    }

    state_ = state;
}

bus_comm::frame_or_error bus_comm::read_frame()
{
    if (!device_.is_readable<raw_frame>())
        return bus_error{"Unable to read frame from device", {}};

    raw_frame frame;
    device_.read_into(frame);

    if (crc16_generator{}(&frame, sizeof(frame))) // If the CRC yields non-zero, then the frame is garbled
        return bus_error{"Invalid CRC", {}};

    if (frame.request)
        return bus_error{"Received request message while expecting a response", {}};

    auto const & job = jobs_.back();
    if (job.frame.address != frame.address)
        return bus_error{"Received response from: '"
            + std::to_string(frame.address)
            + "', expected: '"
            + std::to_string(job.frame.address) + "'", {}};

    auto response_code = static_cast<bus_response_code>(frame.command_or_response);
    auto make_error = [response_code](char const * const what) { return bus_error{what, response_code}; };
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
    // TODO: we might want to keep the relative order of high prio messages

    if (high_prio && !jobs_.empty()) // A job is being handled
        jobs_.insert(jobs_.end() - 1, std::move(j));
    else
        jobs_.push_front(std::move(j));

    if (jobs_.size() == 1)
        do_write();
}

} // End of namespace
