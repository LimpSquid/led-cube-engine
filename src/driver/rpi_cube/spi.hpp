#pragma once

#include <driver/rpi_cube/iodev.hpp>
#include <cube/core/utils.hpp>
#include <cstdint>
#include <linux/spi/spidev.h>

namespace driver::rpi_cube
{

struct spi_config
{
    char const * const device;
    uint8_t mode;
    uint8_t bits_per_word;
    uint32_t max_speed_hz;
};

class spi
    : public iodev
{
public:
    spi(spi_config config, cube::core::engine_context & context);

private:
    spi(spi &) = delete;
    spi(spi &&) = delete;

    std::size_t bytes_avail_for_reading() const override;
    std::size_t bytes_avail_for_writing() const override;
    std::size_t read(void * dst, std::size_t count) override;
    std::size_t write(void const * src, std::size_t count) override;
    void clear_input() override;
    void clear_output() override;

    cube::core::safe_fd fd_;
};

} // End of namespace
