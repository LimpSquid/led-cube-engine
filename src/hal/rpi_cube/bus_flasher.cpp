#include <hal/rpi_cube/bus_flasher.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <hal/rpi_cube/hexfile.hpp>
#include <cube/core/composite_function.hpp>

using namespace cube::core;
namespace fs = std::filesystem;

namespace
{

constexpr uint32_t boot_magic = 0x0b00b1e5;

// See: https://github.com/LimpSquid/led-controller/blob/5-make-a-bootloader/led-controller.X/include/bootloader/bootloader.h#L6
enum class bootloader_query : uint8_t
{
    mem_phy_start   = 0,
    mem_phy_end     = 1,
    mem_word_size   = 2,
    mem_dword_size  = 3,
    mem_row_size    = 4,
    mem_page_size   = 5,
};

} // End of namespace

namespace hal::rpi_cube
{

bus_flasher::bus_flasher(bus_comm & comm) :
    bus_comm_(comm)
{ }

void bus_flasher::flash_hex_file(fs::path const & filepath)
{
    bus_request_params<bus_command::app_exe_cpu_reset> params{}; // Reset immediately

    // Boards that are already in bootloader mode will ignore this command
    bus_comm_.broadcast(std::move(params), std::bind(&bus_flasher::set_boot_magic, this));
}

void bus_flasher::set_boot_magic()
{
    bus_request_params<bus_command::bl_set_boot_magic> params;
    params.magic = boot_magic;

    std::vector<bus_node> all_slaves;
    std::generate_n(all_slaves.begin(), bus_node::num_addresses::value, [n = bus_node::min_address::value] () mutable { return n++; });

    bus_comm_.send_for_all(std::move(params), [&](auto responses) {
        for (auto [slave, response] : responses) {
            if (response)
                bus_slaves_.push_back(slave);
        }

        get_memory_layout();
    }, all_slaves);
}

void bus_flasher::get_memory_layout()
{
    struct session
    {
        std::unordered_map<bus_node, memory_layout> memory_layouts;
    };

    auto s = std::make_shared<session>();

    auto const query_all = [&](bootloader_query query, auto memory_layout::*field) {
        bus_request_params<bus_command::bl_get_info> params;
        params.query = static_cast<uint8_t>(query);

        bus_comm_.send_for_all(std::move(params), [this, s, field](auto responses) {
            for (auto [slave, response] : responses) {
                if (response)
                    s->memory_layouts[slave].*field = response->query_result;
                else
                    mark_failed(slave);
            }

            if (s.use_count() == 1) {
                std::for_each(bus_slaves_failed_.begin(), bus_slaves_failed_.end(),
                    [&](auto const & node) { s->memory_layouts.erase(node); });
                memory_layouts_ = std::move(s->memory_layouts);

                do_erase();
            }
        }, bus_slaves_);
    };

    query_all(bootloader_query::mem_phy_start, &memory_layout::start_address);
    query_all(bootloader_query::mem_phy_end, &memory_layout::end_address);
    query_all(bootloader_query::mem_word_size, &memory_layout::word_size);
    query_all(bootloader_query::mem_row_size, &memory_layout::row_size);
    query_all(bootloader_query::mem_page_size, &memory_layout::page_size);
}

void bus_flasher::do_erase()
{
    bus_comm_.send_for_all<bus_command::bl_exe_erase>({}, [&](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave);
        }

        when_ready(std::bind(&bus_flasher::do_push_word, this));
    }, bus_slaves_);
}

void bus_flasher::do_push_word()
{

}

void bus_flasher::when_ready(std::function<void()> handler)
{
    // TODO: eventually with timeout?
    bus_comm_.send_for_all<bus_command::bl_get_status>({}, [&, h = std::move(handler)](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave);
            else if (!response->bootloader_ready)
                return when_ready(std::move(h));
        }

        h();
    }, bus_slaves_);
}

void bus_flasher::mark_failed(bus_node slave)
{
    auto search = std::find(bus_slaves_.begin(), bus_slaves_.end(), slave);

    if (search != bus_slaves_.end()) {
        bus_slaves_.erase(search);
        memory_layouts_.erase(slave);
        bus_slaves_failed_.push_back(slave);
    }
}

} // End of namespace
