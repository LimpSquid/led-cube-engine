#include <cube/gfx/easing.hpp>
#include <cube/core/math.hpp>

using namespace cube::core;

namespace cube::gfx
{

double linear_curve::operator()(double progress) const
{
    return progress;
}

double sine_curve::operator()(double progress) const
{
    return std::sin(0.5 * M_PI * progress);
}

double bounce_curve::operator()(double progress) const
{
    return std::pow(2, 6 * (progress - 1.0)) * std::abs(std::sin(3.5 * M_PI * progress));
}

}
