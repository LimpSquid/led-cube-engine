#include <hal/rpi_cube/display.hpp>
#include <cube/core/composite_function.hpp>
#include <cube/core/events.hpp>
#include <cube/core/logging.hpp>
#include <iostream>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;

namespace hal::rpi_cube
{

namespace detail
{

struct rgb_buffer
{
    struct red   { using offset = std::integral_constant<std::size_t, 0 * cube::cube_size_2d>; };
    struct green { using offset = std::integral_constant<std::size_t, 1 * cube::cube_size_2d>; };
    struct blue  { using offset = std::integral_constant<std::size_t, 2 * cube::cube_size_2d>; };

    struct slice_buffer
    {
        template<typename C>
        auto begin() { return data.begin() + C::offset::value; }
        template<typename C>
        auto const begin() const { return data.begin() + C::offset::value; }

        template<typename C>
        auto end() { return begin<C>() + cube::cube_size_2d; }
        template<typename C>
        auto const end() const { return begin<C>() + cube::cube_size_2d; }

        std::array<color_t, 3 * cube::cube_size_2d> data;
    };

    std::array<slice_buffer, cube::cube_size_1d> slices;
};
static_assert(sizeof(rgb_buffer) == (3 * sizeof(uint8_t) * cube::cube_size_3d));
static_assert(sizeof(rgb_buffer) == (sizeof(graphics_buffer) - sizeof(color_t) * cube::cube_size_3d)); // Must be same size except alpha channel
static_assert(std::is_trivially_copyable_v<rgb_buffer>);

std::unique_ptr<rgb_buffer const> make_rgb_buffer(graphics_buffer const & buffer)
{
    auto rgbb = std::make_unique<rgb_buffer>();
    auto rgba = buffer.begin();

    for (auto & slice : rgbb->slices) {
        auto r = slice.begin<rgb_buffer::red>();
        auto g = slice.begin<rgb_buffer::green>();
        auto b = slice.begin<rgb_buffer::blue>();

        while (r != slice.end<rgb_buffer::red>()) {
            *r++ = static_cast<color_t>(*rgba   >> 24);
            *g++ = static_cast<color_t>(*rgba   >> 16);
            *b++ = static_cast<color_t>(*rgba++ >>  8);
        }

        assert(g == slice.end<rgb_buffer::green>());
        assert(b == slice.end<rgb_buffer::blue>());
    }

    return rgbb;
}

struct async_pixel_pump
{
    using completion_handler_t = std::function<void()>;

    static std::unique_ptr<async_pixel_pump> run(display & d, std::unique_ptr<rgb_buffer const> b, completion_handler_t h)
    {
        auto pp = std::make_unique<async_pixel_pump>(d, std::move(b), std::move(h));
        pp->run();
        return pp;
    }

    async_pixel_pump(display & d, std::unique_ptr<rgb_buffer const> b, completion_handler_t h) :
        disp(d),
        completion_handler(std::move(h)),
        invoker(disp.context().event_poller, std::bind(signature<>::select_overload(&async_pixel_pump::run), this)),
        buffer(std::move(b)),
        current_slice(0)
    { }

    void run()
    {
        // Writing pixels is a blocking operation, write one slice
        // at the time and allow the event to run in between.

        assert(current_slice < cube::cube_size_1d);
        auto const slave_select = disp.resources_.pixel_comm_ss.begin() + current_slice;
        auto const slave_address = disp.resources_.bus_comm_slave_addresses.begin() + current_slice;

        auto const search = disp.detected_slaves_.find(*slave_address);
        if (search != disp.detected_slaves_.end()) {
            hal::rpi_cube::gpio_lo_guard gpio_guard{*slave_select};
            disp.resources_.pixel_comm_device.write_from(buffer->slices[current_slice]);
        }

        if (++current_slice == cube::cube_size_1d)
            completion_handler();
        else
            invoker.schedule();
    }

    display & disp;
    completion_handler_t completion_handler;
    function_invoker invoker;
    std::unique_ptr<rgb_buffer const> buffer;
    int current_slice;
};

display_shutdown_signal::display_shutdown_signal(display & display) :
    engine_shutdown_signal(display.context()),
    display_(display)
{ }

void display_shutdown_signal::shutdown_requested()
{
    display_.pixel_pump_.reset(); // Immediately stop sending pixels
    display_.bus_comm_.send_for_all<bus_command::app_exe_clear>({},
        std::bind(&display_shutdown_signal::ready_for_shutdown, this),
        display_.resources_.bus_comm_slave_addresses
    );
}

constexpr seconds bus_monitor_interval{5};
constexpr std::size_t max_queue_size{3};

} // End of namespace

display::display(engine_context & context) :
    graphics_device(context),
    bus_monitor_(context, std::bind(&display::probe_slaves, this), true),
    resources_(context),
    bus_comm_(resources_.bus_comm_device),
    shutdown_signal_(*this)
{
    bus_monitor_.start(detail::bus_monitor_interval);
}

display::~display()
{ }

int display::map_to_offset(int x, int y, int z) const
{
    return x + z * cube::cube_size_1d + y * cube::cube_size_2d;
}

void display::show(graphics_buffer const & buffer)
{
    if (buffer_queue_.size() == detail::max_queue_size) {
        LOG_WRN_PERIODIC(1s, "Too many buffers in queue, dropping oldest...");
        buffer_queue_.pop_back();
    }

    buffer_queue_.push_front(detail::make_rgb_buffer(buffer));
    if (!pixel_pump_)
        pixel_pump_run();
}

void display::pixel_pump_run()
{
    if (buffer_queue_.empty()) {
        pixel_pump_.reset();
        return;
    }

    pixel_pump_ = detail::async_pixel_pump::run(*this, std::move(buffer_queue_.back()), std::bind(&display::pixel_pump_finished, this));
    buffer_queue_.pop_back();
}

void display::pixel_pump_finished()
{
    bus_comm_.broadcast<bus_command::app_exe_dma_reset>({}, std::bind(&display::pixel_pump_run, this));
}

void display::probe_slaves()
{
    bus_comm_.send_for_each<bus_command::app_get_status>({}, [this](auto slave, auto response) {
        if (!response) {
            detected_slaves_.erase(slave);

            LOG_WRN("Failed to probe slave", LOG_ARG("address", as_hex(slave)));
            return;
        }

        // Already detected the slave, check status
        if (detected_slaves_.count(slave)) {
            if (!response->layer_dma_error)
                return;

            // If we add more error flags, introduce a generic log message
            detected_slaves_.erase(slave);
            LOG_WRN("DMA error, reinitializing slave", LOG_ARG("address", as_hex(slave)));
        }

        // Slave detected, (re)initialize
        auto [sys_version_handler, dma_reset_handler] = decompose_function([this, slave](
            bus_response_params_or_error<bus_command::app_get_version> version_response,
            bus_response_params_or_error<bus_command::app_exe_dma_reset> reset_response) {
                if (!version_response || ! reset_response) {
                    LOG_WRN("Failed to initialize slave", LOG_ARG("address", as_hex(slave)));
                    return;
                }

                std::string version =
                    "v" + std::to_string(version_response->major) +
                    "." + std::to_string(version_response->minor) +
                    "." + std::to_string(version_response->patch);

                LOG_INF("Initialized slave",
                    LOG_ARG("address", as_hex(slave)),
                    LOG_ARG("version", version));
                detected_slaves_.insert(slave);
            }
        );

        bus_comm_.send<bus_command::app_get_version>({}, slave, std::move(sys_version_handler));
        bus_comm_.send<bus_command::app_exe_dma_reset>({}, slave, std::move(dma_reset_handler));
    }, resources_.bus_comm_slave_addresses);
}

} // End of namespace
