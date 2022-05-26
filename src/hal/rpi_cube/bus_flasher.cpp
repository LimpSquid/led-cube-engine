#include <hal/rpi_cube/bus_flasher.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <hal/rpi_cube/hexfile.hpp>
#include <hal/rpi_cube/crc.hpp>
#include <cube/core/logging.hpp>
#include <cassert>

using namespace cube::core;
namespace fs = std::filesystem;

namespace
{

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
        stream(std::move(s))
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

// TODO list:
// - We need to fix ALL the handlers, its possible that the bus_flasher is destroyed while a request from
//   the bus_comm is still holding a reference to the destroyed class (e.g. the function that will be
//   ran on completion of a request). I think we should be able to wrap each handler which then makes use
//   of `make_parent_tracker()` from cube/core/utils.hpp.

namespace hal::rpi_cube
{

bus_flasher::bus_flasher(bus_comm & comm) :
    bus_comm_(comm)
{ }

void bus_flasher::flash_hex_file(fs::path const & filepath)
{
    hex_filepath_ = filepath; // TODO: check existence?
    reset_nodes();

    LOG_DBG("Flashing hex file", LOG_ARG("filepath", filepath.c_str()));
}

void bus_flasher::reset_nodes()
{
    bus_request_params<bus_command::app_exe_cpu_reset> params{}; // Reset immediately

    // Nodes that are already in bootloader mode will ignore this command
    bus_comm_.broadcast(std::move(params), std::bind(&bus_flasher::set_boot_magic, this));

    LOG_DBG("Broadcasting CPU reset");
}

void bus_flasher::set_boot_magic()
{
    bus_request_params<bus_command::bl_set_boot_magic> params;
    params.magic = 0x0b00b1e5;

    std::vector<bus_node> all_slaves;
    std::generate_n(all_slaves.begin(), bus_node::num_addresses::value, [n = bus_node::min_address::value] () mutable { return n++; });

    bus_comm_.send_for_all(std::move(params), [this](auto responses) {
        for (auto [slave, response] : responses) {
            if (response) {
                nodes_.push_back({slave, flashing_in_progress, {}, {}});
                LOG_DBG("Boot magic provided", LOG_ARG("address", as_hex(slave.address)));
            }
        }

        when_ready(std::bind(&bus_flasher::get_memory_layout, this));
    }, all_slaves);

    LOG_DBG("Sending boot magic to lock-in bootloader mode");
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
                    mark_failed(slave, response.error().what);
            }

            if (s.use_count() == 1)
                flash_erase();
        }, view(nodes_).filter(state_filter<flashing_in_progress>{})
                       .transform(extract_member<bus_node>{})
                       .get());
    };

    query_for_all(bootloader_query::mem_phy_start, &memory_layout::start_address);
    query_for_all(bootloader_query::mem_phy_end, &memory_layout::end_address);
    query_for_all(bootloader_query::mem_word_size, &memory_layout::word_size);
    query_for_all(bootloader_query::mem_row_size, &memory_layout::row_size);
    query_for_all(bootloader_query::mem_page_size, &memory_layout::page_size);

    LOG_DBG("Querying memory layout");
}

void bus_flasher::flash_erase()
{
    bus_comm_.send_for_all<bus_command::bl_exe_erase>({}, [this](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
        }

        when_ready(std::bind(&bus_flasher::flash_next_group, this));
    }, view(nodes_).filter(state_filter<flashing_in_progress>{})
                   .transform(extract_member<bus_node>{})
                   .get());

    LOG_DBG("Erasing application flash region");
}

void bus_flasher::flash_next_group()
{
    auto mem_layouts = view(nodes_).filter(state_filter<flashing_in_progress>{})
                                   .transform(extract_member<memory_layout>{})
                                   .get();
    if (mem_layouts.empty())
        return; // TODO: done, now boot all boards!

    auto layout = mem_layouts.front();
    auto blob = parse_hex_file(hex_filepath_, layout); // Also checks if the memory layout is valid
    if (!blob) {
        LOG_DBG("Unable to prepare HEX file for group", LOG_ARG("memory_layout", to_string(layout)));

        auto nodes = view(nodes_).filter(state_filter<flashing_in_progress>{})
                                 .filter(memory_layout_filter{layout})
                                 .transform(extract_member<bus_node>{})
                                 .get();
        std::for_each(nodes.begin(), nodes.end(), std::bind(&bus_flasher::mark_failed, this, std::placeholders::_1, blob.error().what));
        return flash_next_group();
    }

    LOG_DBG("Prepared HEX file for group",
        LOG_ARG("memory_layout", to_string(layout)),
        LOG_ARG("blob_size", blob->size()));

    auto group = std::make_shared<group_t const>(
        std::move(*blob),
        view<node_cref_t>({nodes_.begin(), nodes_.end()})
            .filter(state_filter<flashing_in_progress>{})
            .filter(memory_layout_filter{layout})
            .get());
    push_blob(std::move(group));
}

void bus_flasher::push_blob(std::shared_ptr<group_t const> group)
{
    assert(group);
    auto const & [_, nodes] = *group;

    bus_comm_.send_for_all<bus_command::bl_exe_row_reset>({}, [this, group](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
        }

        push_row(group, 0);
    }, view(nodes).transform(extract_member<bus_node>{}).get());

    LOG_DBG("Pushing blob to group", LOG_ARG("group_size", nodes.size()));
}

void bus_flasher::push_row(std::shared_ptr<group_t const> group, uint32_t row)
{
    assert(group);
    auto const & [blob, nodes] = *group;
    uint32_t const word_size = *std::get<memory_layout>(nodes.front().get()).word_size;
    uint32_t const row_size = *std::get<memory_layout>(nodes.front().get()).row_size;
    uint32_t const n_rows = blob.size() / row_size;

    if (word_size > sizeof(bus_request_params<bus_command::bl_exe_push_word>))
        throw std::runtime_error("Unable to handle word size: " + std::to_string(word_size));

    if (row >= n_rows)
        return flash_next_group();

    crc16_generator crc;
    auto data = blob.begin() + row * row_size;
    auto advance_word = [&]()
    {
        bus_request_params<bus_command::bl_exe_push_word> params;
        for (uint32_t i = 0; i < word_size; ++i)
            params.data[i] = *data++; // Little-endian
        crc(params.data, word_size);
        return params;
    };

    // Notice that we're also broadcasting to slaves that are not part of the current group.
    // This is no problem since we're not going to instruct these slaves to burn the row to flash.
    // You might ask, what if a slave that is receiving this broadcast has a different memory layout,
    // e.g. a smaller row size, won't that cause any problems? Well, when a slave receives more words
    // than fits in a single row, it will return a bus error. However, since this is a broadcast
    // errors like these will be silently ignored, which is is just what we need :-).

    // Broadcasts with the same command params will always be sent out in the same order
    // as the requests were initially made. When the completion handler of the last broadcast
    // is executed, we can be sure that previous broadcasts have all been sent out.
    for (uint32_t i = 0; i < (row_size - word_size); i += word_size)
        bus_comm_.broadcast(advance_word());
    bus_comm_.broadcast(advance_word(), std::bind(&bus_flasher::verify_row, this, group, row, crc));
    assert(std::distance(blob.begin(), data) == row_size);

    LOG_DBG("Pushing row to group",
        LOG_ARG("group_size", nodes.size()),
        LOG_ARG("row", row),
        LOG_ARG("n_rows", n_rows));
}

void bus_flasher::verify_row(std::shared_ptr<group_t const> group, uint32_t row, uint16_t crc)
{
    assert(group);
    auto const & [_, nodes] = *group;

    bus_comm_.send_for_all<bus_command::bl_get_row_crc>({}, [this, group, row, crc](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response) // TODO: eventually we could retry this specific row for the slaves that have an incorrect CRC?
                mark_failed(slave, response.error().what);
            else if (response->crc != crc)
                mark_failed(slave, "Row CRC did not match");
        }

        burn_row(group, row);
    }, view(nodes).filter(state_filter<flashing_in_progress>{})
                  .transform(extract_member<bus_node>{})
                  .get());

    LOG_DBG("Verifying row CRC",
        LOG_ARG("group_size", nodes.size()),
        LOG_ARG("row", row));
}

void bus_flasher::burn_row(std::shared_ptr<group_t const> group, uint32_t row)
{
    assert(group);
    auto const & [_, nodes] = *group;

    bus_comm_.send_for_all<bus_command::bl_exe_row_burn>({}, [this, group, row](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
        }

        when_ready(std::bind(&bus_flasher::push_row, this, group, row + 1));
    }, view(nodes).filter(state_filter<flashing_in_progress>{})
                  .transform(extract_member<bus_node>{})
                  .get());

    LOG_DBG("Burning row to flash",
        LOG_ARG("group_size", nodes.size()),
        LOG_ARG("row", row));
}

void bus_flasher::when_ready(std::function<void()> handler)
{
    // TODO: eventually with timeout? Mark slave failed if it never became ready
    bus_comm_.send_for_all<bus_command::bl_get_status>({}, [&, h = std::move(handler)](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
            else if (response->bootloader_error)
                mark_failed(slave, "Bootloader error");
            else if (!response->bootloader_ready)
                return when_ready(std::move(h));
        }

        h();
    }, view(nodes_).filter(state_filter<flashing_in_progress>{})
                   .transform(extract_member<bus_node>{})
                   .get());
}

bus_flasher::node_t & bus_flasher::find_or_throw(bus_node const & slave)
{
    auto search = std::find_if(nodes_.begin(), nodes_.end(),
        [&](node_t const & node) { return std::get<bus_node>(node) == slave; });

    if (search == nodes_.cend())
        throw std::runtime_error("Unable to find node with address: " + std::to_string(slave.address));
    return *search;
}

void bus_flasher::mark_failed(bus_node const & slave, std::string const & desc)
{
    node_t & node = find_or_throw(slave);
    std::get<flashing_state>(node) = flashing_failed;
    std::get<std::string>(node) = desc;
    LOG_DBG("Flashing failed for node",
        LOG_ARG("address", as_hex(slave.address)),
        LOG_ARG("description", desc));
}

void bus_flasher::mark_succeeded(bus_node const & slave)
{
    std::get<flashing_state>(find_or_throw(slave)) = flashing_succeeded;
    LOG_DBG("Flashing succeeded for node", LOG_ARG("address", as_hex(slave.address)));
}

} // End of namespace
