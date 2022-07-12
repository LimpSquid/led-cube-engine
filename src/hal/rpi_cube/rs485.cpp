#include <hal/rpi_cube/rs485.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/utils.hpp>
#include <sys/file.h>
#include <thread>
#include <sched.h>
#include <iostream>

using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

constexpr std::size_t buffer_size{4096};

struct critical_section_guard
{
    critical_section_guard() :
        restore_policy(sched_getscheduler(0))
    {
        // https://linux.die.net/man/2/sched_setscheduler
        sched_param param{};
        param.sched_priority = sched_get_priority_max(SCHED_FIFO);
        sched_setscheduler(0, SCHED_FIFO, &param);
    }

    ~critical_section_guard()
    {
        sched_param param{}; // The sched_priority parameter must be specified as 0;
        sched_setscheduler(0, restore_policy, &param);
    }

    critical_section_guard(critical_section_guard &) = delete;
    critical_section_guard(critical_section_guard &&) = delete;

    int const restore_policy;
};

template<typename F>
void critical_section(F func)
{
    critical_section_guard g{};
    func();
}

safe_fd open_or_throw(hal::rpi_cube::rs485_config const & config)
{
    safe_fd fd = ::open(config.device , O_RDWR);
    if (fd < 0)
        throw std::runtime_error("Unable to open device: "s + config.device);

    int lock = ::flock(fd, LOCK_EX | LOCK_NB);
    if (lock < 0)
        throw std::runtime_error("Unable to lock device: "s + config.device);

    termios tty;
    if (::tcgetattr(fd, &tty) < 0)
        throw std::runtime_error("Unable to read tty attributes for device: "s + config.device);

    // See https://linux.die.net/man/3/termios
    tty.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE); // Disable hardware flow control, disable parity, one stop bit and clear data size
    tty.c_cflag |= (CS8 | CLOCAL | CREAD); // Eight data bits, ignore model control lines and enable receiver
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG); // Disable canonical mode, echo, new-line echo and signal interpretation
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~(OPOST | ONLCR); // Prevent special interpretation of output bytes
    tty.c_cc[VTIME] = 0; // Make read() return immediately
    tty.c_cc[VMIN] = 0;

    ::cfsetispeed(&tty, config.baudrate);
    ::cfsetospeed(&tty, config.baudrate);

    if (::tcsetattr(fd, TCSANOW, &tty) < 0)
        throw std::runtime_error("Unable to write tty attributes for device: "s + config.device);
    return fd;
}

unsigned int speed_or_throw(speed_t baudrate)
{
    switch (baudrate) {
#define CASE(speed) \
    case B##speed: return speed;
        CASE(9600)
        CASE(19200)
        CASE(38400)
        CASE(57600)
        CASE(115200)
        CASE(576000)
#undef CASE
        default:
            throw std::runtime_error("Unknown baudrate:" + std::to_string(baudrate));
    }
}

microseconds approximate_transmission_time(speed_t baudrate, unsigned int num_of_chars)
{
    constexpr double bits_per_char = 10.0;
    return microseconds{static_cast<microseconds::rep>(((bits_per_char * num_of_chars) / speed_or_throw(baudrate)) * 1000000LU + 0.5)}; // Ceil
}

template<typename H>
auto synchronize(std::mutex & lock, H handler)
{
    std::lock_guard guard(lock);
    return handler();
}

void safe_join(std::thread & t)
{
    if (t.joinable())
        t.join();
}

template<typename H>
void safe_start(std::thread & t, H && h)
{
    safe_join(t);
    t = std::thread(std::move(h));
}

} // End of namespace

namespace hal::rpi_cube
{

rs485::rs485(rs485_config config, engine_context & context) :
    iodev(context, config.device),
    fd_(open_or_throw(config)),
    event_notifier_(context.event_poller, fd_, fd_event_notifier::read | fd_event_notifier::error, [this](auto evs) { on_event(evs); }),
    tx_buffer_(buffer_size),
    rx_buffer_(buffer_size),
    dir_gpio_(config.dir_pin, gpio::output),
    draining_(false)
{
    dir_gpio_.write(gpio::lo);
}

rs485::~rs485()
{
    safe_join(drain_thread_);
}

std::size_t rs485::bytes_avail_for_reading() const
{
    return rx_buffer_.size();
}

std::size_t rs485::bytes_avail_for_writing() const
{
    return tx_buffer_.capacity() - tx_buffer_.size();
}

std::size_t rs485::read(void * dst, std::size_t count)
{
    bool const was_full = rx_buffer_.full();
    std::size_t size = 0;
    char * out = reinterpret_cast<char *>(dst);

    while (!rx_buffer_.empty() && size != count) {
        *out++ = rx_buffer_.back();
        size++;
        rx_buffer_.pop_back();
    }

    if (was_full && size)
        synchronize(lock_, [this]() { event_notifier_.set_events(fd_event_notifier::read); }); // Now we can read data into the buffer again
    return size;
}

std::size_t rs485::write(void const * src, std::size_t count)
{
    bool const was_empty = tx_buffer_.empty();
    std::size_t size = 0;
    char const * in = reinterpret_cast<char const *>(src);

    while (!tx_buffer_.full() && size != count) {
        tx_buffer_.push_front(*in++);
        size++;
    }

    if (was_empty && size)
        synchronize(lock_, [this]() { event_notifier_.set_events(fd_event_notifier::write); }); // Now we can write data to device
    return size;
}

void rs485::clear_input()
{
    if (::tcflush(fd_, TCIFLUSH) < 0)
        throw_errno("rs485 input flush");
    rx_buffer_.clear();
}

void rs485::clear_output()
{
    if (::tcflush(fd_, TCOFLUSH) < 0)
        throw_errno("rs485 output flush");
    tx_buffer_.clear();
}

void rs485::on_event(fd_event_notifier::event_flags evs)
{
    if (evs & fd_event_notifier::error)
        throw_errno();
    if (evs & fd_event_notifier::read)
        read_into_buffer();
    if (evs & fd_event_notifier::write)
        write_from_buffer();
}

void rs485::read_into_buffer()
{
    std::size_t const capacity = rx_buffer_.capacity() - rx_buffer_.size();

    // User must read from the buffer first
    if (!capacity)
        return synchronize(lock_, [this]() { event_notifier_.clr_events(fd_event_notifier::read); });

    auto size = ::read(fd_, &chunk_buffer_, std::min(sizeof(chunk_buffer_), capacity));
    if (size < 0)
        throw_errno("rs485 read");
    if (size == 0)
        return;

    char const * data = chunk_buffer_;
    while (size--)
        rx_buffer_.push_front(*data++);
    notify_readable();
}

void rs485::write_from_buffer()
{
    // This whole construct sucks balls, wrong choices were made in the hardware,
    // as we could've perfectly used the RTS pin of the hardware flow control.
    // Since I would like to avoid patching this in the hardware we have to deal with
    // it here. In the future we might want to take it a step further if this user space
    // solution doesn't suffice, and that is to patch the serial driver and do the RS485
    // direction toggling over there.

    bool const empty = tx_buffer_.empty();
    bool const can_transfer = synchronize(lock_, [this, empty]() { // User must write data to the buffer first
        if (empty || draining_) {
            event_notifier_.clr_events(fd_event_notifier::write);
            return false;
        }
        return true;
    });

    if (!can_transfer) {
        if (empty)
            notify_transfer_complete();
        return;
    }

    // At this point no locks are necessary as we are sure that the drain thread is not running

    termios tty;
    if (::tcgetattr(fd_, &tty) < 0)
        throw std::runtime_error("Unable to read tty attributes from filedescriptor.");

    std::size_t csize;
    char * data = chunk_buffer_;
    for (csize = 0; csize < sizeof(chunk_buffer_) && !tx_buffer_.empty(); ++csize) {
        *data++ = tx_buffer_.back();
        tx_buffer_.pop_back();
    }

    steady_clock::time_point transmission_end;
    gpio_hi_guard dir_guard{dir_gpio_};
    ssize_t size;

    critical_section([&]() {
        transmission_end = steady_clock::now(); // Mark starting time
        size = ::write(fd_, &chunk_buffer_, csize);
    });

    if (size < 0)
        throw_errno("rs485 write");
    if (static_cast<std::size_t>(size) != csize)
        throw std::runtime_error("Chunk size too large, unable to write to underlying device");

    transmission_end += approximate_transmission_time(::cfgetospeed(&tty), static_cast<unsigned int>(size)); // Add approximate time to transmit the characters
    draining_ = true;

    safe_start(drain_thread_, [this,
        guard = std::move(dir_guard),
        transmission_end = std::move(transmission_end)]() mutable {
            // Because tcdrain only polls every couple of milliseconds, we can't use that as the direction would be pulled
            // low way too late. For now make a rough estimate of how long it would take to transfer the amount of chars
            // we've just written to the device and sleep for atleast that amount. This is far from ideal but should work
            // fine for now.
            auto const now = steady_clock::now();
            if (now < transmission_end) {
                auto const sleep = duration_cast<microseconds>(transmission_end - now).count();
                usleep(static_cast<useconds_t>(sleep));
            }
            guard.restore(); // GPIO access is mutually exclusive with main thread, so no lock necessary

            synchronize(lock_, [this]() {
                event_notifier_.set_events(fd_event_notifier::write); // Underlying epoll_ctl is thread safe w.r.t. our main thread polling the event_poller
                draining_ = false;
            });
        }
    );
}

} // End of namespace
