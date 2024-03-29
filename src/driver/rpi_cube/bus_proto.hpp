#pragma once

#include <type_traits>
#include <cstdint>
#include <functional>
#include <string>

namespace driver::rpi_cube
{

// See: https://github.com/LimpSquid/led-controller/blob/master/led-controller.X/source/[app|bootloader]/bus_func_impl.c
enum class bus_command : unsigned char
{
    // Application commands start at '0'
    app_get_status              = 3,
    app_get_version             = 5,

    app_set_auto_buffer_swap    = 0,

    app_exe_led_open_detection  = 1,
    app_exe_dma_reset           = 2,
    app_exe_dma_swap_buffers    = 4,
    app_exe_cpu_reset           = 6, // TODO (in led controller firmware): the board should send out the reply before resetting!
    app_exe_clear               = 7,

    // Bootloader commands start at '128'
    bl_get_status               = 128,
    bl_get_info                 = 129,
    bl_get_version              = 137,
    bl_get_row_crc              = 136,

    bl_set_boot_magic           = 131,

    bl_exe_erase                = 130,
    bl_exe_row_reset            = 133,
    bl_exe_push_word            = 134,
    bl_exe_row_burn             = 135,
    bl_exe_boot                 = 132,
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

struct bus_node
{
    using min_address = std::integral_constant<unsigned char, 0>;
    using max_address = std::integral_constant<unsigned char, 31>;
    using num_addresses = std::integral_constant<unsigned char, max_address::value - min_address::value>;

    bus_node() :
        address(0)
    { }

    bus_node(unsigned char addr) :
        address(addr & max_address())
    { }

    bool operator==(bus_node const & other) const
    {
        return address == other.address;
    }

    unsigned char address   :5;
    unsigned char           :3;
};

struct low_prio_request
{
    using high_prio = std::integral_constant<bool, false>;
    using response_timeout = std::integral_constant<std::size_t, 10>; // In milliseconds
};

struct high_prio_request
{
    using high_prio = std::integral_constant<bool, true>;
    using response_timeout = std::integral_constant<std::size_t, 10>; // In milliseconds
};

struct bootloader_request
{
    using high_prio = std::integral_constant<bool, false>;
    using response_timeout = std::integral_constant<std::size_t, 50>; // In milliseconds
};

template<bus_command>
struct bus_request_params : low_prio_request { };
template<bus_command>
struct bus_response_params { };

template<> struct bus_request_params<bus_command::app_exe_dma_swap_buffers> : high_prio_request { };

template<>
struct bus_request_params<bus_command::app_exe_cpu_reset> : low_prio_request
{
    int32_t delay_ms;
};

template<>
struct bus_request_params<bus_command::app_set_auto_buffer_swap> : low_prio_request
{
    bus_request_params(bool e) :
        enable(e)
    { }

    bool enable;
};

template<> struct bus_request_params<bus_command::bl_get_status> : bootloader_request { };
template<> struct bus_request_params<bus_command::bl_get_version> : bootloader_request { };
template<> struct bus_request_params<bus_command::bl_get_row_crc> : bootloader_request { };
template<> struct bus_request_params<bus_command::bl_exe_erase> : bootloader_request { };
template<> struct bus_request_params<bus_command::bl_exe_row_reset> : bootloader_request { };
template<> struct bus_request_params<bus_command::bl_exe_boot> : bootloader_request { };

template<>
struct bus_request_params<bus_command::bl_set_boot_magic> : bootloader_request
{
    uint32_t magic;
};

template<>
struct bus_request_params<bus_command::bl_get_info> : bootloader_request
{
    uint8_t query;
};

template<>
struct bus_request_params<bus_command::bl_exe_push_word> : bootloader_request
{
    uint8_t data[4];
};

template<>
struct bus_request_params<bus_command::bl_exe_row_burn> : bootloader_request
{
    uint32_t phy_address;
};

template<>
struct bus_response_params<bus_command::app_get_version>
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

template<>
struct bus_response_params<bus_command::app_get_status>
{
    bool layer_ready            :1;
    bool layer_dma_error        :1;
    bool layer_auto_buffer_swap :1;
};

template<>
struct bus_response_params<bus_command::bl_get_status>
{
    bool bootloader_ready               :1;
    bool bootloader_error               :1;
    bool bootloader_waiting_for_magic   :1;
};

template<> struct bus_response_params<bus_command::bl_get_info>
{
    uint32_t query_result;
};

template<>
struct bus_response_params<bus_command::bl_get_version>
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

template<>
struct bus_response_params<bus_command::bl_get_row_crc>
{
    uint16_t crc;
};

inline std::string to_string(bus_response_params<bus_command::bl_get_status> const & response)
{
    using std::operator""s;

    return "["s
         + "ready: " + std::to_string(response.bootloader_ready) + ", "
         + "error: " + std::to_string(response.bootloader_error) + ", "
         + "waiting_for_magic: " + std::to_string(response.bootloader_waiting_for_magic) + "]";
}

inline std::string to_string(bus_response_params<bus_command::app_get_status> const & response)
{
    using std::operator""s;

    return "["s
         + "layer_ready: " + std::to_string(response.layer_ready) + ", "
         + "layer_auto_buffer_swap: " + std::to_string(response.layer_auto_buffer_swap) + ", "
         + "layer_dma_error: " + std::to_string(response.layer_dma_error) + "]";
}

} // End of namespace

template<>
struct std::hash<driver::rpi_cube::bus_node>
{
    std::size_t operator()(driver::rpi_cube::bus_node const & node) const noexcept
    {
        return std::hash<decltype(node.address)>{}(node.address);
    }
};
