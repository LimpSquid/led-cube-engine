#pragma once

#include <cstdint>

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

template<bus_command>
struct bus_request_params { };
template<bus_command>
struct bus_response_params { };

template<>
struct bus_request_params<bus_command::exe_sys_cpu_reset>
{
    int32_t delay;
};

template<>
struct bus_response_params<bus_command::get_layer_ready>
{
    bool ready;
};

template<>
struct bus_response_params<bus_command::get_dma_ready_to_recv>
{
    bool ready;
};

template<>
struct bus_response_params<bus_command::get_sys_version>
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

} // End of namespace
