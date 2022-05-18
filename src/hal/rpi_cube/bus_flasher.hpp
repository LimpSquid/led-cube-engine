#pragma once

#include <hal/rpi_cube/bus_proto.hpp>
#include <hal/rpi_cube/hexfile.hpp>
#include <functional>

namespace hal::rpi_cube
{

class bus_comm;
class bus_flasher
{
public:
    bus_flasher(bus_comm & comm);

    void flash_hex_file(std::filesystem::path const & filepath);

private:
    bus_flasher(bus_flasher &) = delete;
    bus_flasher(bus_flasher &&) = delete;

    void set_boot_magic();
    void get_memory_layout();
    void do_erase();
    void do_push_word();

    void when_ready(std::function<void()> handler);
    void mark_failed(bus_node slave);

    bus_comm & bus_comm_;
    std::vector<bus_node> bus_slaves_;
    std::vector<bus_node> bus_slaves_failed_;
    std::unordered_map<bus_node, memory_layout> memory_layouts_;
};

} // End of namespace
