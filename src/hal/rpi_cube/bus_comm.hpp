#pragma once

#include <hal/rpi_cube/iodev.hpp>
#include <cube/core/expected.hpp>
#include <cube/core/timers.hpp>
#include <deque>
#include <cstring>

namespace hal::rpi_cube
{

// See: https://github.com/LimpSquid/led-controller/blob/master/led-controller.X/source/bus_func_impl.c
enum class bus_command : unsigned char
{
    get_layer_ready         = 0,
    get_dma_ready_to_recv   = 3,
    get_sys_version         = 5,

    exe_led_open_detection  = 1,
    exe_dma_reset           = 2,
    exe_dma_swap_buffers    = 4,
    exe_sys_cpu_reset       = 6, // Fixme: a bit of a special case as this may not send a response with a small timeout!
};

enum class bus_response_code : unsigned char
{
    ok = 0,

    err_unknown = 100,
    err_again,
    err_invalid_payload,
    err_invalid_command,
};
static_assert(sizeof(bus_command) == sizeof(bus_response_code));

cube::core::void_or_error verify_response_code(bus_response_code response);

template<bus_command>
struct request_params { };
template<bus_command>
struct response_params { };

template<>
struct response_params<bus_command::get_sys_version>
{
    unsigned char major;
    unsigned char minor;
    unsigned char patch;
};

template<>
struct response_params<bus_command::get_layer_ready>
{
    bool ready;
};

struct bus_node
{
    unsigned char address   :5;
    unsigned char           :3;
};

class iodev;
class bus_comm
{
public:
    template<bus_command C>
    using response_handler_t = std::function<void(cube::core::expected_or_error<response_params<C>>)>;

    enum bus_state
    {
        idle,
        transfer,
        error,
    };

    bus_comm(iodev & device);

    template<bus_command C>
    void send(request_params<C> params, bus_node const & target, response_handler_t<C> response_handler)
    {
        static_assert(std::is_trivially_copyable_v<request_params<C>>);
        static_assert(std::is_trivially_copyable_v<response_params<C>>);
        static_assert(sizeof(request_params<C>) <= sizeof(payload_t));
        static_assert(sizeof(response_params<C>) <= sizeof(payload_t));

        bus_frame frame{};
        frame.request = true;
        frame.address = target.address;
        frame.command_or_response = static_cast<unsigned char>(C);
        std::memcpy(frame.payload.data(), &params, sizeof(params));
        frame.crc = 0x0000; // Todo: crc

        add_job({
            std::move(frame),
            [h = std::move(response_handler)](cube::core::expected_or_error<bus_frame> frame) {
                if (!frame)
                    return h(frame.error());
                auto const rc = verify_response_code(static_cast<bus_response_code>(frame->command_or_response));
                if (!rc)
                    return h(rc.error());
                response_params<C> params;
                std::memcpy(&params, frame->payload.data(), sizeof(params));
                h(std::move(params));
            }
        });
    }

    template<bus_command C>
    void broadcast(request_params<C> params)
    {
        constexpr unsigned char broadcast_address{32};
        static_assert(std::is_trivially_copyable_v<request_params<C>>);
        static_assert(std::is_trivially_copyable_v<response_params<C>>);
        static_assert(sizeof(request_params<C>) <= sizeof(payload_t));
        static_assert(sizeof(response_params<C>) <= sizeof(payload_t));

        bus_frame frame{};
        frame.request = true;
        frame.address = broadcast_address;
        frame.command_or_response = static_cast<unsigned char>(C);
        std::memcpy(frame.payload.data(), &params, sizeof(params));
        frame.crc = 0x0000; // Todo: crc

        add_job({std::move(frame), nullptr});
    }

private:
    using payload_t = std::array<unsigned char, 4>;

    struct [[gnu::packed]] bus_frame
    {
        unsigned char request   :1;
        unsigned char address   :6;
        unsigned char           :1;
        unsigned char command_or_response;
        payload_t payload;
        unsigned short crc; // Todo: crc16_t
    };
    static_assert(sizeof(bus_frame) == 8);

    struct job
    {
        bus_frame frame;
        std::function<void(cube::core::expected_or_error<bus_frame>)> handler;
    };

    void do_read();
    void do_write();
    void do_write_one();
    void do_timeout();
    void switch_state(bus_state state);

    cube::core::expected_or_error<bus_frame> read_frame();

    void add_job(job && j);

    iodev & device_;
    iodev_subscription read_subscription_;

    cube::core::single_shot_timer transfer_watchdog_;
    std::deque<job> jobs_;
    bus_state state_;
};

} // End of namespace
