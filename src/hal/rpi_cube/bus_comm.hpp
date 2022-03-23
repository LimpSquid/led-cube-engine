#pragma once

#include <hal/rpi_cube/bus_proto.hpp>
#include <hal/rpi_cube/iodev.hpp>
#include <hal/rpi_cube/crc.hpp>
#include <cube/core/expected.hpp>
#include <cube/core/timers.hpp>
#include <cube/core/enum.hpp>
#include <deque>
#include <cstring>
#include <variant>

namespace hal::rpi_cube
{

SIMPLE_NS_ENUM(bus_state,
    idle,
    transfer,
    error
)

struct bus_error :
    cube::core::unexpected_error
{
    std::optional<bus_response_code> response_code; // If not set we encountered a CRC error, timeout, etc.
};

template<bus_command C>
using bus_response_params_or_error = cube::core::basic_expected<bus_response_params<C>, bus_error>;

class bus_comm
{
public:
    template<bus_command C>
    using response_handler_t = std::function<void(bus_response_params_or_error<C>)>;
    using broadcast_handler_t = std::function<void()>;

    bus_comm(iodev & device);

    template<bus_command C>
    void send(bus_request_params<C> params, bus_node const & target, response_handler_t<C> response_handler)
    {
        static_assert(std::is_trivially_copyable_v<bus_request_params<C>>);
        static_assert(std::is_trivially_copyable_v<bus_response_params<C>>);
        static_assert(sizeof(bus_request_params<C>) <= sizeof(payload_t));
        static_assert(sizeof(bus_response_params<C>) <= sizeof(payload_t));

        raw_frame frame{};
        frame.request = true;
        frame.address = target.address;
        frame.command_or_response = static_cast<unsigned char>(C);
        std::memcpy(frame.payload.data(), &params, sizeof(params));
        frame.crc = crc16_generator{}(&frame, sizeof(frame) - sizeof(frame.crc));

        add_job({std::move(frame),
            unicast_params{
                [h = std::move(response_handler)](frame_or_error frame) {
                    if (!frame)
                        return h(frame.error());

                    bus_response_params<C> params;
                    std::memcpy(&params, frame->payload.data(), sizeof(params));
                    h(std::move(params));
                }
            }
        }, bus_request_params<C>::high_prio::value);
    }

    template<bus_command C>
    void broadcast(bus_request_params<C> params, broadcast_handler_t handler = nullptr)
    {
        constexpr unsigned char broadcast_address{32};
        static_assert(std::is_trivially_copyable_v<bus_request_params<C>>);
        static_assert(std::is_trivially_copyable_v<bus_response_params<C>>);
        static_assert(sizeof(bus_request_params<C>) <= sizeof(payload_t));
        static_assert(sizeof(bus_response_params<C>) <= sizeof(payload_t));

        raw_frame frame{};
        frame.request = true;
        frame.address = broadcast_address;
        frame.command_or_response = static_cast<unsigned char>(C);
        std::memcpy(frame.payload.data(), &params, sizeof(params));
        frame.crc = crc16_generator{}(&frame, sizeof(frame) - sizeof(frame.crc));

        add_job({std::move(frame),
            broadcast_params{std::move(handler)}
        }, bus_request_params<C>::high_prio::value);
    }

private:
    using payload_t = std::array<unsigned char, 4>;
    using crc16_t = crc16_generator::value_type;

    struct [[gnu::packed]] raw_frame
    {
        unsigned char request   :1;
        unsigned char address   :6;
        unsigned char           :1;
        unsigned char command_or_response;
        payload_t payload;
        crc16_t crc;
    };
    static_assert(sizeof(raw_frame) == 8);
    static_assert(std::is_trivially_copyable_v<raw_frame>);

    using frame_or_error = cube::core::basic_expected<raw_frame, bus_error>;

    struct unicast_params
    {
        std::function<void(frame_or_error)> handler;
        unsigned int attempt{0};
    };

    struct broadcast_params
    {
        broadcast_handler_t handler;
    };

    struct job
    {
        raw_frame frame;
        std::variant<
            unicast_params,
            broadcast_params
        > params;
    };


    void do_read();
    void do_write();
    void do_write_one();
    void do_timeout();
    void do_finish();
    void switch_state(bus_state state);

    frame_or_error read_frame();
    void add_job(job && j, bool high_prio = false);

    iodev & device_;
    iodev_subscription read_subscription_;

    cube::core::single_shot_timer response_watchdog_;
    std::deque<job> jobs_;
    bus_state state_;
};

} // End of namespace
