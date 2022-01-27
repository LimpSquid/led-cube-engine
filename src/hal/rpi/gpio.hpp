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

    gpio(unsigned int pin, direction dir);
    ~gpio();

private:
    unsigned int pin_;
    bool exported_;
};

} // End of namespace
