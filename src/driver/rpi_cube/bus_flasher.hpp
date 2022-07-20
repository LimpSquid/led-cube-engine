#pragma once

#include <driver/rpi_cube/bus_proto.hpp>
#include <driver/rpi_cube/hexfile.hpp>
#include <functional>

namespace driver::rpi_cube
{

class bus_comm;
class bus_flasher
{
public:
    bus_flasher(bus_comm & comm);

    void flash_hex_file(std::filesystem::path const & filepath);

private:
    enum flashing_state
    {
        flashing_in_progress,
        flashing_failed,
        flashing_succeeded
    };

    using node_t = std::tuple<bus_node, flashing_state, memory_layout, std::string /* reason of failure */>;
    using node_cref_t = std::reference_wrapper<node_t const>;
    // Nodes in a group all have the same memory layout, thus the blob can
    // be pushed to the nodes via broadcasts instead of point-to-point.
    using group_t = std::tuple<memory_blob, std::vector<node_cref_t>>;

    template<typename T>
    struct extract_member { T operator()(node_t const & node) const { return std::get<T>(node); } };
    template<flashing_state S>
    struct state_filter { bool operator()(node_t const & node) const { return std::get<flashing_state>(node) != S; } };
    struct memory_layout_filter
    {
        bool operator()(node_t const & node) const { return std::get<memory_layout>(node) == layout; }

        memory_layout layout;
    };

    bus_flasher(bus_flasher &) = delete;
    bus_flasher(bus_flasher &&) = delete;

    template<typename H>
    void when_ready(H handler, std::optional<std::vector<node_cref_t>> opt_nodes = {});
    node_t & find_or_throw(bus_node const & slave);
    void mark_failed(bus_node const & slave, std::string const & desc = "Unknown error");
    void mark_succeeded(bus_node const & slave);

    void reset_nodes();
    void set_boot_magic();
    void get_memory_layout();

    void flash_erase();
    void flash_next_group();
    void push_blob(std::shared_ptr<group_t const> group);
    void push_row(std::shared_ptr<group_t const> group, uint32_t row);
    void verify_row(std::shared_ptr<group_t const> group, uint32_t row, uint16_t crc);
    void burn_row(std::shared_ptr<group_t const> group, uint32_t row);
    void boot(std::shared_ptr<group_t const> group);

    bus_comm & bus_comm_;
    std::vector<node_t> nodes_;
    std::filesystem::path hex_filepath_;
};

} // End of namespace
