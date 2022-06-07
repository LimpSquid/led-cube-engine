#include <hal/rpi_cube/spi.hpp>
#include <sys/file.h>
#include <sys/ioctl.h>

using namespace cube::core;
using std::operator""s;

namespace
{

safe_fd open_or_throw(hal::rpi_cube::spi_config const & config)
{
    safe_fd fd = ::open(config.device , O_WRONLY);
    if (fd < 0)
        throw std::runtime_error("Unable to open device: "s + config.device);

    int lock = ::flock(fd, LOCK_EX | LOCK_NB);
    if (lock < 0)
        throw std::runtime_error("Unable to lock device: "s + config.device);

    if (::ioctl(fd, SPI_IOC_WR_MODE, &config.mode) < 0)
        throw std::runtime_error("Unable to write SPI mode to device: "s + config.device);

    if (::ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &config.bits_per_word) < 0)
        throw std::runtime_error("Unable to write SPI bits per word to device: "s + config.device);

    if (::ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &config.max_speed_hz) < 0)
        throw std::runtime_error("Unable to write SPI max speed to device: "s + config.device);

    return fd;
}

} // End of namespace

namespace hal::rpi_cube
{

spi::spi(spi_config config, engine_context & context) :
    iodev(context, config.device),
    fd_(open_or_throw(config))
{ }

std::size_t spi::bytes_avail_for_reading() const
{
    return 0; // Write only
}

std::size_t spi::bytes_avail_for_writing() const
{
    return std::numeric_limits<uint32_t>::max();
}

std::size_t spi::read(void *, std::size_t)
{
    throw std::runtime_error("SPI device is write-only");
    return 0;
}

std::size_t spi::write(void const * src, std::size_t count)
{
    spi_ioc_transfer msg{};

    msg.tx_buf = reinterpret_cast<uint64_t>(src);
    msg.len = static_cast<uint32_t>(count & std::numeric_limits<uint32_t>::max());

    int64_t const size = ::ioctl(fd_, SPI_IOC_MESSAGE(1), &msg);
	if (size < 0)
        throw_errno("SPI write");
    return static_cast<std::size_t>(size);
}

void spi::clear_input()
{ }

void spi::clear_output()
{ }

} // End of namespace
