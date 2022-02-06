#include <hal/rpi_cube/rs485.hpp>
#include <cube/core/engine_context.hpp>
#include <sys/file.h>

using namespace cube::core;
using std::operator""s;

namespace
{

hal::rpi_cube::safe_fd open_or_throw(hal::rpi_cube::rs485_config const & config)
{
    hal::rpi_cube::safe_fd fd = ::open(config.device , O_RDWR);
    if (fd < 0)
        throw std::runtime_error("Unable to open device: "s + config.device);

    int lock = ::flock(fd, LOCK_EX | LOCK_NB);
    if (lock < 0)
        throw std::runtime_error("Unable to lock device: "s + config.device);

    termios tty;
    if (tcgetattr(fd, &tty) < 0)
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

    cfsetispeed(&tty, config.baudrate);
    cfsetospeed(&tty, config.baudrate);

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        throw std::runtime_error("Unable to write tty attributes for device: "s + config.device);

    return fd;
}

} // End of namespace

namespace hal::rpi_cube
{

rs485::rs485(rs485_config config, cube::core::engine_context & context) :
    fd_(open_or_throw(config)),
    event_notifier_(context.event_poller, fd_, fd_event_notifier::read),
    dir_gpio_(config.dir_pin, gpio::output)
{ }

} // End of namespace
