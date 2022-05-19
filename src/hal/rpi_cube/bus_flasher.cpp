#include <hal/rpi_cube/bus_flasher.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <hal/rpi_cube/hexfile.hpp>

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

template<typename T>
struct view
{
    view(std::vector<T> s) :
        stream(s)
    { }

    std::vector<T> & get() { return stream; }
    std::vector<T> const & get() const { return stream; }
    operator std::vector<T> & () { return stream; }
    operator std::vector<T> const & () const { return stream; }

    template<typename F>
    view & filter(F predicate)
    {
        std::remove_if(stream.begin(), stream.end(), std::move(predicate));
        return *this;
    }

    template<typename F, typename R = decltype(std::declval<F>()(std::declval<T>()))>
    view<R> transform(F predicate) const
    {
        std::vector<R> transformed;
        std::transform(stream.begin(), stream.end(), std::back_inserter(transformed), std::move(predicate));
        return transformed;
    }

    std::vector<T> stream;
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
                nodes_.push_back({slave, flashing_busy, memory_layout{}});
        }

        get_memory_layout();
    }, all_slaves);
}

void bus_flasher::get_memory_layout()
{
    struct session {};
    auto s = std::make_shared<session>();

    auto const query_for_all = [&](bootloader_query query, auto memory_layout::*field) {
        bus_request_params<bus_command::bl_get_info> params;
        params.query = static_cast<uint8_t>(query);

        bus_comm_.send_for_all(std::move(params), [this, s, field](auto responses) {
            for (auto [slave, response] : responses) {
                if (response)
                    std::get<memory_layout>(find_or_throw(slave)).*field = response->query_result;
                else
                    mark_failed(slave);
            }

            if (s.use_count() == 1)
                flash_erase();
        }, view(nodes_).filter(state_filter<flashing_busy>{})
                       .transform(extract_member<bus_node>{})
                       .get());
    };

    query_for_all(bootloader_query::mem_phy_start, &memory_layout::start_address);
    query_for_all(bootloader_query::mem_phy_end, &memory_layout::end_address);
    query_for_all(bootloader_query::mem_word_size, &memory_layout::word_size);
    query_for_all(bootloader_query::mem_row_size, &memory_layout::row_size);
    query_for_all(bootloader_query::mem_page_size, &memory_layout::page_size);
}

void bus_flasher::flash_erase()
{
    bus_comm_.send_for_all<bus_command::bl_exe_erase>({}, [&](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave);
        }

        when_ready(std::bind(&bus_flasher::flash_next_group, this));
    }, view(nodes_).filter(state_filter<flashing_busy>{})
                   .transform(extract_member<bus_node>{})
                   .get());
}

void bus_flasher::flash_next_group()
{

}

void bus_flasher::when_ready(std::function<void()> handler)
{
    // TODO: eventually with timeout? Mark slave failed if it never became ready
    bus_comm_.send_for_all<bus_command::bl_get_status>({}, [&, h = std::move(handler)](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave);
            else if (!response->bootloader_ready)
                return when_ready(std::move(h));
        }

        h();
    }, view(nodes_).filter(state_filter<flashing_busy>{})
                   .transform(extract_member<bus_node>{})
                   .get());
}

bus_flasher::node_t & bus_flasher::find_or_throw(bus_node slave)
{
    auto search = std::find_if(nodes_.begin(), nodes_.end(),
        [&](node_t const & node) { return std::get<bus_node>(node) == slave; });

    if (search == nodes_.cend())
        throw std::runtime_error("Unable to find node with address: " + std::to_string(slave.address));
    return *search;
}

void bus_flasher::mark_failed(bus_node slave)
{
    std::get<flashing_state>(find_or_throw(slave)) = flashing_failed;
}

void bus_flasher::mark_succeeded(bus_node slave)
{
    std::get<flashing_state>(find_or_throw(slave)) = flashing_succeeded;
}

} // End of namespace
