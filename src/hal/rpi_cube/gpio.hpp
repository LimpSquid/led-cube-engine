#pragma once

#include <algorithm>

namespace hal::rpi_cube
{

class gpio
{
public:
    enum direction
    {
        input,
        output,
    };

    enum level
    {
        lo = 0,
        hi = 1,
    };

    gpio(unsigned int pin, direction dir);
    gpio(gpio && other);
    ~gpio();

    level read() const;
    void write(level lvl) const;

private:
    gpio(gpio & other) = delete;

    void unexport() const;

    unsigned int pin_;
    bool exported_;
};

inline gpio make_output(unsigned int pin) { return {pin, gpio::output}; }
inline gpio make_input(unsigned int pin) { return {pin, gpio::input}; }

template<gpio::level L>
class basic_gpio_guard
{
public:
    constexpr static gpio::level acquisition_level = L;
    constexpr static gpio::level restore_level = (L == gpio::hi) ? gpio::lo : gpio::hi;

    basic_gpio_guard(gpio const & io) :
        io_(&io)
    {
        io_->write(acquisition_level);
    }

    basic_gpio_guard(basic_gpio_guard && other) :
        io_(other.io_)
    {
        other.io_ = nullptr;
    }

    ~basic_gpio_guard()
    {
        restore();
    }

    basic_gpio_guard & operator=(basic_gpio_guard && other)
    {
        restore();
        std::swap(io_, other.io_);
    }

    void restore()
    {
        if (io_) {
            io_->write(restore_level);
            io_ = nullptr;
        }
    }

private:
    basic_gpio_guard(basic_gpio_guard &) = delete;

    gpio const * io_;
};

using gpio_hi_guard = basic_gpio_guard<gpio::hi>;
using gpio_lo_guard = basic_gpio_guard<gpio::lo>;

} // End of namespace
