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
    get_sys_version         = 5,
    get_dma_ready_to_recv   = 3,

    exe_led_open_detection  = 1,
    exe_dma_reset           = 2,
    exe_dma_swap_buffers    = 4,
    exe_sys_cpu_reset       = 6, // Fixme: a bit of a special case as this may not send a response with a small timeout!
};

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
    unsigned char address;
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

        payload_t params_raw = {0};
        std::memcpy(params_raw.data(), &params, sizeof(params));
        jobs_.push_front({std::move(target), C, std::move(params_raw),
            [h = std::move(response_handler)](cube::core::expected_or_error<payload_t> params_raw) {
                if (!params_raw)
                    return h(params_raw.error());
                response_params<C> params;
                std::memcpy(&params, params_raw->data(), sizeof(params));
                h(std::move(params));
            }
        });

        if (jobs_.size() == 1)
            do_write();
    }

private:
    using payload_t = std::array<unsigned char, 4>;

    struct job
    {
        bus_node target;
        bus_command command;
        payload_t params_raw;
        std::function<void(cube::core::expected_or_error<payload_t>)> handler;
    };

    iodev & device_;
    iodev_subscription read_subscription_;

    void do_read();
    void do_write();
    void do_write_one();
    void do_timeout();
    void switch_state(bus_state state);

    cube::core::single_shot_timer timeout_timer_;
    std::deque<job> jobs_;
    bus_state state_;
};

} // End of namespace
