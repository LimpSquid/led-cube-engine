#pragma once

// pull in some math libraries from cpp standard
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace cube::core
{

template<typename T>
struct Range
{
    constexpr Range(T const & f, T const & t) :
        from(f),
        to(t)
    { }

    T from;
    T to;
};

template<typename TIn, typename TOut>
constexpr inline TOut map(
    TIn const & value,
    Range<TIn> const & in_range,
    Range<TOut> const & out_range)
{
    return out_range.from + (out_range.to - out_range.from) * (value - in_range.from) / (in_range.to - in_range.from);
}

template<typename TIn, typename TOut>
constexpr inline TOut map(
    TIn const & value,
    TIn const &  in_from, TIn const & in_to,
    TOut const & out_from, TOut const &  out_to)
{
    return map(value, Range(in_from, in_to), Range(out_from, out_to));
}

constexpr inline bool equal(double lhs, double rhs)
{
    return std::abs(lhs - rhs) <= ( (std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * DBL_EPSILON);
}

constexpr inline bool equal(float lhs, float rhs)
{
    return std::abs(lhs - rhs) <= ( (std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * FLT_EPSILON);
}

constexpr inline bool greater_than(double lhs, double rhs)
{
    return (lhs - rhs) > ( (std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * DBL_EPSILON);
}

constexpr inline bool greater_than(float lhs, float rhs)
{
    return (lhs - rhs) > ( (std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * FLT_EPSILON);
}

constexpr inline bool less_than(double lhs, double rhs)
{
    return (rhs - lhs) > ( (std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * DBL_EPSILON);
}

constexpr inline bool less_than(float lhs, float rhs)
{
    return (rhs - lhs) > ( (std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * FLT_EPSILON);
}

} // End of namespace
