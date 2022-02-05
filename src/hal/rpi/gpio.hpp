#pragma once

#include <optional>

namespace hal::rpi
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

} // End of namespace
