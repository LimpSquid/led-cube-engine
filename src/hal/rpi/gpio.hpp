#pragma once

#include <boost/noncopyable.hpp>
#include <optional>

namespace hal::rpi
{

class gpio :
    boost::noncopyable
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
    ~gpio();

    level read() const;
    void write(level lvl) const;

private:
    void unexport() const;

    unsigned int pin_;
    bool exported_;
};

} // End of namespace
