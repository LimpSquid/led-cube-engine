#pragma once

namespace cube::util
{

class color
{
public:
    color();
    color(unsigned char red, unsigned char green, unsigned char blue);
    color(const color &other) = default;
    ~color() = default;

    color &operator=(const color &other) = default;

private:
    unsigned char red_;
    unsigned char green_;
    unsigned char blue_;
};

}