#include <driver/rpi_cube/bus_flasher.hpp>
#include <driver/rpi_cube/bus_comm.hpp>
#include <driver/rpi_cube/hexfile.hpp>
#include <driver/rpi_cube/crc.hpp>
#include <cube/core/logging.hpp>
#include <numeric>
#include <cassert>

using namespace cube::core;
namespace fs = std::filesystem;
using std::operator""s;

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
class view
{
public:
    view(std::vector<T> stream) :
        stream_(std::move(stream))
    { }

    std::vector<T> & get() { return stream_; }
    std::vector<T> const & get() const { return stream_; }
    operator std::vector<T> & () { return stream_; }
    operator std::vector<T> const & () const { return stream_; }

    template<typename F>
    view & filter(F predicate)
    {
        stream_.erase(std::remove_if(stream_.begin(), stream_.end(), std::move(predicate)), stream_.end());
        return *this;
    }

    template<typename F, typename R = decltype(std::declval<F>()(std::declval<T>()))>
    view<R> transform(F predicate) const
    {
        std::vector<R> transformed;
        std::transform(stream_.begin(), stream_.end(), std::back_inserter(transformed), std::move(predicate));
        return transformed;
    }

private:
    std::vector<T> stream_;
};

} // End of namespace

namespace driver::rpi_cube
{

bus_flasher::bus_flasher(bus_comm & comm, completion_handler_t handler) :
    bus_comm_(comm),
    completion_handler_(std::move(handler)),
    scope_tracker_(make_scope_tracker())
{ }

void bus_flasher::flash_hex_file(fs::path const & filepath)
{
    // Flashing busy
    if (!nodes_.empty())
        return;

    LOG_DBG("Started to flash hex file", LOG_ARG("filepath", filepath.c_str()));

    for (auto addr = bus_node::min_address::value; addr != bus_node::max_address::value; ++addr)
        nodes_.push_back({addr, flashing_in_progress, {}, {}});
    hex_filepath_ = filepath;
    reset_nodes();
}

template<typename H>
void bus_flasher::when_ready(H handler, std::optional<std::vector<node_cref_t>> opt_nodes)
{
    LOG_DBG_PERIODIC(100ms, "Waiting for nodes to become ready");

    // TODO: eventually with timeout? Mark slave failed if it never became ready
    auto nodes = opt_nodes
        ? std::move(opt_nodes.value())
        : std::vector<node_cref_t>{nodes_.begin(), nodes_.end()};

    send_for_all<bus_command::bl_get_status>({}, [this, h = std::move(handler), nodes](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
            else if (response->bootloader_error)
                mark_failed(slave, "Bootloader error");
            else if (!response->bootloader_ready) {
                LOG_DBG_PERIODIC(100ms, "Node not ready", LOG_ARG("address", as_hex(slave.address)));
                return when_ready(std::move(h), std::move(nodes));
            }
        }

        LOG_DBG("Nodes ready");
        h();
    }, view(std::move(nodes)).filter(state_filter<flashing_in_progress>{})
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

void bus_flasher::mark_not_detected(bus_node const & slave)
{
    node_t & node = find_or_throw(slave);
    std::get<flashing_state>(node) = flashing_failed_not_detected;

    LOG_DBG("Node not detected", LOG_ARG("address", as_hex(slave.address)));
}

void bus_flasher::mark_succeeded(bus_node const & slave)
{
    std::get<flashing_state>(find_or_throw(slave)) = flashing_succeeded;

    LOG_DBG("Flashing succeeded for node", LOG_ARG("address", as_hex(slave.address)));
}

void bus_flasher::reset_nodes()
{
    LOG_DBG("Broadcasting CPU reset");

    // Nodes that are already in bootloader mode will ignore this command
    broadcast<bus_command::app_exe_cpu_reset>({}, std::bind(&bus_flasher::set_boot_magic, this));
}

void bus_flasher::set_boot_magic()
{
    LOG_DBG("Sending boot magic to lock-in bootloader mode");

    bus_request_params<bus_command::bl_set_boot_magic> params;
    params.magic = 0x0b00b1e5;

    send_for_all(std::move(params), [this](auto responses) {
        for (auto [slave, response] : responses) {
            if (response)
                LOG_DBG("Boot magic provided", LOG_ARG("address", as_hex(slave.address)));
            else
                mark_not_detected(slave);
        }

        when_ready(std::bind(&bus_flasher::get_memory_layout, this));
    }, view(nodes_).filter(state_filter<flashing_in_progress>{})
                   .transform(extract_member<bus_node>{})
                   .get());
}

void bus_flasher::get_memory_layout()
{
    LOG_DBG("Querying memory layout");

    struct session {};
    auto s = std::make_shared<session>();

    auto const query_for_all = [&](bootloader_query query, auto memory_layout::*field) {
        bus_request_params<bus_command::bl_get_info> params;
        params.query = static_cast<uint8_t>(query);

        send_for_all(std::move(params), [this, s, field](auto responses) {
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
}

void bus_flasher::flash_erase()
{
    LOG_DBG("Erasing application flash region");

    send_for_all<bus_command::bl_exe_erase>({}, [this](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
        }

        when_ready(std::bind(&bus_flasher::flash_next_group, this));
    }, view(nodes_).filter(state_filter<flashing_in_progress>{})
                   .transform(extract_member<bus_node>{})
                   .get());
}

void bus_flasher::flash_next_group()
{
    auto mem_layouts = view(nodes_).filter(state_filter<flashing_in_progress>{})
                                   .transform(extract_member<memory_layout>{})
                                   .get();
    if (mem_layouts.empty())
        return complete();

    auto layout = mem_layouts.front();
    auto blob = parse_hex_file(hex_filepath_, layout); // Also checks if the memory layout is valid
    if (!blob) {
        LOG_DBG("Unable to prepare HEX file for group", LOG_ARG("memory_layout", to_string(layout)));

        auto nodes = view(nodes_).filter(state_filter<flashing_in_progress>{})
                                 .filter(memory_layout_filter{layout})
                                 .transform(extract_member<bus_node>{})
                                 .get();
        for (auto && node : nodes)
            mark_failed(node, blob.error().what);
        return flash_next_group();
    }

    LOG_DBG("Prepared HEX file for group",
        LOG_ARG("memory_layout", to_string(layout)),
        LOG_ARG("blob_size", blob->size()));

    auto group = std::make_shared<group_state_t const>(
        std::move(*blob),
        view<node_cref_t>({nodes_.begin(), nodes_.end()})
            .filter(state_filter<flashing_in_progress>{})
            .filter(memory_layout_filter{layout})
            .get());
    push_blob(std::move(group));
}

void bus_flasher::push_blob(std::shared_ptr<group_state_t const> group)
{
    assert(group);
    auto const & [_, nodes] = *group;

    LOG_DBG("Pushing blob to group", LOG_ARG("group_size", nodes.size()));

    send_for_all<bus_command::bl_exe_row_reset>({}, [this, group](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
        }

        push_row(group, 0);
    }, view(nodes).transform(extract_member<bus_node>{}).get());
}

void bus_flasher::push_row(std::shared_ptr<group_state_t const> group, uint32_t row)
{
    assert(group);
    auto const & [blob, nodes] = *group;
    uint32_t const word_size = *std::get<memory_layout>(nodes.front().get()).word_size;
    uint32_t const row_size = *std::get<memory_layout>(nodes.front().get()).row_size;
    uint32_t const n_rows = blob.size() / row_size;

    if (word_size > sizeof(bus_request_params<bus_command::bl_exe_push_word>))
        return error("Unable to handle word size: "s + std::to_string(word_size));

    LOG_DBG("Pushing row to group",
        LOG_ARG("row", row),
        LOG_ARG("n_rows", n_rows));

    if (row >= n_rows)
        return boot(std::move(group));

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
        broadcast(advance_word());
    broadcast(advance_word(), std::bind(&bus_flasher::verify_row, this, group, row, crc));
    assert(std::distance(blob.begin() + row * row_size, data) == row_size);
}

void bus_flasher::verify_row(std::shared_ptr<group_state_t const> group, uint32_t row, uint16_t crc)
{
    LOG_DBG("Verifying row CRC", LOG_ARG("row", row));

    assert(group);
    auto const & [_, nodes] = *group;

    send_for_all<bus_command::bl_get_row_crc>({}, [this, group, row, crc](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
            else if (response->crc != crc) // TODO: eventually we could retry this specific row for the slaves that have an incorrect CRC?
                mark_failed(slave, "Row CRC did not match");
        }

        burn_row(group, row);
    }, view(nodes).filter(state_filter<flashing_in_progress>{})
                  .transform(extract_member<bus_node>{})
                  .get());
}

void bus_flasher::burn_row(std::shared_ptr<group_state_t const> group, uint32_t row)
{
    LOG_DBG("Burning row to flash", LOG_ARG("row", row));

    assert(group);
    auto const & [_, nodes] = *group;
    uint32_t const start_address = std::get<memory_layout>(nodes.front().get()).start_address;
    uint32_t const row_size = *std::get<memory_layout>(nodes.front().get()).row_size;

    bus_request_params<bus_command::bl_exe_row_burn> params;
    params.phy_address = start_address + row * row_size;

    send_for_all<bus_command::bl_exe_row_burn>(std::move(params), [this, group, row, nodes](auto responses) {
        for (auto [slave, response] : responses) {
            if (!response)
                mark_failed(slave, response.error().what);
        }

        when_ready(std::bind(&bus_flasher::push_row, this, group, row + 1), std::move(nodes));
    }, view(nodes).filter(state_filter<flashing_in_progress>{})
                  .transform(extract_member<bus_node>{})
                  .get());
}

void bus_flasher::boot(std::shared_ptr<group_state_t const> group)
{
    LOG_DBG("Booting boards");

    assert(group);
    auto const & [blob, nodes] = *group;

    send_for_all<bus_command::bl_exe_boot>({}, [this](auto responses) {
        for (auto [slave, response] : responses) {
            if (response)
                mark_succeeded(slave); // TODO: maybe we should check if it actually succeeded
            else
                mark_failed(slave, response.error().what);
        }

        flash_next_group();
    }, view(nodes).filter(state_filter<flashing_in_progress>{})
                  .transform(extract_member<bus_node>{})
                  .get());
}

template<bus_command C>
void bus_flasher::broadcast(bus_request_params<C> && params)
{
    if (view(nodes_).filter(state_filter<flashing_in_progress>{}).get().empty())
        return complete();

    bus_comm_.broadcast(std::forward<decltype(params)>(params));
}

template<bus_command C, typename H>
void bus_flasher::broadcast(bus_request_params<C> && params, H && handler)
{
    if (view(nodes_).filter(state_filter<flashing_in_progress>{}).get().empty())
        return complete();

    bus_comm_.broadcast(std::forward<decltype(params)>(params), [h = std::forward<H>(handler), r = make_weak_ref(scope_tracker_)](auto && ... args){
        if (is_valid(r))
            h(std::forward<decltype(args)>(args)...);
    });
}

template<bus_command C, typename H, typename N>
void bus_flasher::send_for_all(bus_request_params<C> && params, H && handler, N && nodes)
{
    if (view(nodes_).filter(state_filter<flashing_in_progress>{}).get().empty())
        return complete();

    bus_comm_.send_for_all(std::forward<decltype(params)>(params), [h = std::forward<H>(handler), r = make_weak_ref(scope_tracker_)](auto && ... args){
        if (is_valid(r))
            h(std::forward<decltype(args)>(args)...);
    }, std::forward<N>(nodes));
}

void bus_flasher::complete()
{
    if (completion_handler_) {
        bus_flasher_result result;

        for (auto && node : nodes_) {
            bus_node const addr = std::get<bus_node>(node);
            flashing_state const state = std::get<flashing_state>(node);
            std::string fail_reason = std::get<std::string>(node);

            switch (state) {
                case flashing_succeeded:    result.flashed_nodes.push_back(addr);                           break;
                case flashing_failed:       result.failed_nodes.push_back({addr, std::move(fail_reason)});  break;
                default:;
            }
        }

        completion_handler_(std::move(result));
    }
    nodes_.clear();
}

void bus_flasher::error(std::string desc)
{
    if (completion_handler_)
        completion_handler_(unexpected_error{std::move(desc)});
    nodes_.clear();
}

} // End of namespace
