#include <hal/rpi_cube/display.hpp>
#include <cube/core/composite_function.hpp>
#include <cube/core/events.hpp>
#include <cube/core/logging.hpp>
#include <iostream>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr seconds bus_monitor_interval{5};
constexpr milliseconds cpu_reset_delay{100};

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

rgb_buffer transform(graphics_buffer const & buffer)
{
    rgb_buffer rgbb;
    auto rgba = buffer.begin();

    for (auto & slice : rgbb.slices) {
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

} // End of namespace

namespace hal::rpi_cube
{

namespace detail
{

struct async_pixel_pump
{
    using completion_handler_t = std::function<void()>;

    static std::unique_ptr<async_pixel_pump> run(display & d, graphics_buffer const & b, completion_handler_t h)
    {
        auto pp = std::make_unique<async_pixel_pump>(d, b, std::move(h));
        pp->run();
        return pp;
    }

    async_pixel_pump(display & d, graphics_buffer const & b, completion_handler_t h) :
        disp(d),
        completion_handler(std::move(h)),
        invoker(disp.context().event_poller, std::bind(signature<>::select_overload(&async_pixel_pump::run), this)),
        buffer(transform(b)),
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
            disp.resources_.pixel_comm_device.write_from(buffer.slices[current_slice]);
        }

        if (++current_slice == cube::cube_size_1d)
            completion_handler();
        else
            invoker.schedule();
    }

    display & disp;
    completion_handler_t completion_handler;
    function_invoker invoker;
    rgb_buffer buffer;
    int current_slice;
};


display_shutdown_signal::display_shutdown_signal(display & display) :
    engine_shutdown_signal(display.context()),
    display_(display)
{ }

void display_shutdown_signal::shutdown_requested()
{
    display_.pixel_pump_.reset(); // Immediately stop sending pixels
    display_.send_for_all<bus_command::exe_layer_clear>({}, std::bind(&display_shutdown_signal::ready_for_shutdown, this));
}

} // End of namespace

display::display(engine_context & context) :
    graphics_device(context),
    bus_monitor_(context, std::bind(&display::probe_slaves, this), true),
    resources_(context),
    bus_comm_(resources_.bus_comm_device),
    shutdown_signal_(*this)
{
    bus_monitor_.start(bus_monitor_interval);
}

display::~display()
{

}

int display::map_to_offset(int x, int y, int z) const
{
    return x + z * cube::cube_size_1d + y * cube::cube_size_2d;
}

void display::show(graphics_buffer const & buffer)
{
    // Another pixel pump still running, eventually queue the buffers?
    if (pixel_pump_) {
        LOG_DBG_PERIODIC(10s, "Ignoring new graphics buffer, pixel pump is still running",
            LOG_ARG("current_slice", pixel_pump_->current_slice));
        return;
    }

    pixel_pump_ = detail::async_pixel_pump::run(*this, buffer, std::bind(&display::pixel_pump_finished, this));
}

void display::pixel_pump_finished()
{
    bus_comm_.broadcast<bus_command::exe_dma_swap_buffers>({}, [&]() { pixel_pump_.reset(); });
}

void display::probe_slaves()
{
    send_for_each<bus_command::get_status>({}, [this](auto slave, auto response) {
        if (!response) {
            detected_slaves_.erase(slave.address);

            LOG_WRN("Failed to probe slave", LOG_ARG("address", as_hex(slave.address)));
            return;
        }

        // Already detected the slave, check status
        if (detected_slaves_.count(slave.address)) {
            if (!response->layer_dma_error)
                return;

            // If we add more error flags, introduce a generic log message
            detected_slaves_.erase(slave.address);
            LOG_WRN("DMA error, reinitializing slave", LOG_ARG("address", as_hex(slave.address)));
        }

        // Slave detected, (re)initialize
        auto [sys_version_handler, dma_reset_handler] = decompose_function([this, slave](
            bus_response_params_or_error<bus_command::get_sys_version> version_response,
            bus_response_params_or_error<bus_command::exe_dma_reset> reset_response) {
                if (!version_response || ! reset_response) {
                    LOG_WRN("Failed to initialize slave", LOG_ARG("address", as_hex(slave.address)));
                    return;
                }

                std::string version =
                    "v" + std::to_string(version_response->major) +
                    "." + std::to_string(version_response->minor) +
                    "." + std::to_string(version_response->patch);

                LOG_INF("Initialized slave",
                    LOG_ARG("address", as_hex(slave.address)),
                    LOG_ARG("version", version));
                detected_slaves_.insert(slave.address);
            }
        );

        bus_comm_.send<bus_command::get_sys_version>({}, slave, std::move(sys_version_handler));
        bus_comm_.send<bus_command::exe_dma_reset>({}, slave, std::move(dma_reset_handler));
    });
}

} // End of namespace
