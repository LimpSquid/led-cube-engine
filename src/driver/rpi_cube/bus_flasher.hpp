#pragma once

#include <driver/rpi_cube/bus_proto.hpp>
#include <driver/rpi_cube/hexfile.hpp>
#include <cube/core/expected.hpp>
#include <cube/core/utils.hpp>
#include <functional>

namespace driver::rpi_cube
{

struct bus_flasher_result
{
    std::vector<bus_node> flashed_nodes;
    std::vector<std::pair<bus_node, std::string>> failed_nodes;
};

class bus_comm;
class bus_flasher
{
public:
    using completion_handler_t = std::function<void(cube::core::expected_or_error<bus_flasher_result>)>;

    bus_flasher(bus_comm & comm, completion_handler_t handler = {});

    void flash_hex_file(std::filesystem::path const & filepath);

private:
    enum flashing_state
    {
        flashing_in_progress,
        flashing_failed,
        flashing_failed_not_detected,
        flashing_succeeded,
    };

    using node_t = std::tuple<bus_node, flashing_state, memory_layout, std::string /* reason of failure */>;
    using node_cref_t = std::reference_wrapper<node_t const>;
    // Nodes in a group all have the same memory layout, thus the blob can
    // be pushed to the nodes via broadcasts instead of point-to-point.
    using group_state_t = std::tuple<memory_blob, std::vector<node_cref_t>>;

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
    void mark_not_detected(bus_node const & slave);
    void mark_succeeded(bus_node const & slave);

    void reset_nodes();
    void set_boot_magic();
    void get_memory_layout();

    void flash_erase();
    void flash_next_group();
    void push_blob(std::shared_ptr<group_state_t const> group);
    void push_row(std::shared_ptr<group_state_t const> group, uint32_t row);
    void verify_row(std::shared_ptr<group_state_t const> group, uint32_t row, uint16_t crc);
    void burn_row(std::shared_ptr<group_state_t const> group, uint32_t row);
    void boot(std::shared_ptr<group_state_t const> group);

    template<bus_command C>
    void broadcast(bus_request_params<C> && params);
    template<bus_command C, typename H>
    void broadcast(bus_request_params<C> && params, H && handler);
    template<bus_command C, typename H, typename N>
    void send_for_all(bus_request_params<C> && params, H && handler, N && nodes);

    void complete();
    void error(std::string desc);

    bus_comm & bus_comm_;
    std::vector<node_t> nodes_;
    std::filesystem::path hex_filepath_;
    completion_handler_t completion_handler_;
    cube::core::scope_tracker_t scope_tracker_;
};

} // End of namespace
