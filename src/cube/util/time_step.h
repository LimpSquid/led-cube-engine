#pragma once

namespace cube::util
{

class time_step
{
public:
    enum time_unit
    {
        tu_milliseconds = 0,
    };

    time_step(const time_unit &unit = tu_milliseconds);
    time_step() = default;

    double sec() const;
    double ms() const;

    void update(double time);

private:
    time_unit unit_;
    double elapsed_;
    double previous_;
};

}