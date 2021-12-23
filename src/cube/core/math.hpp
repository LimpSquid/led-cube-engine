#pragma once

// pull in some math libraries from cpp standard
#include <algorithm>
#include <cmath>
#include <limits>

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

template<typename T>
constexpr Range<T> operator*(Range<T> const & lhs, Range<T> const & rhs) { return {lhs.from * rhs.from, lhs.to * rhs.to}; }
template<typename T>
constexpr Range<T> operator*(Range<T> const & lhs, double scalar) { return {lhs.from * scalar, lhs.to * scalar}; }

template<typename TIn, typename TOut>
constexpr inline TOut map(
    TIn const & value,
    Range<TIn> const & in_range,
    Range<TOut> const & out_range)
{
    if constexpr(std::is_integral_v<TOut> && std::is_floating_point_v<TIn>)
        return out_range.from + std::round((out_range.to - out_range.from) * (value - in_range.from) / (in_range.to - in_range.from));
    else
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

template<typename T>
constexpr inline bool within_range(T const & value, Range<T> const & range)
{
    return (value >= range.from && value <= range.to);
}

template<typename T>
constexpr inline bool equal(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return std::abs(lhs - rhs) <= ((std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * std::numeric_limits<T>::epsilon());
}

template<typename T>
constexpr inline bool greater_than(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return (lhs - rhs) > ((std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * std::numeric_limits<T>::epsilon());
}

template<typename T>
constexpr inline bool less_than(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return (rhs - lhs) > ((std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * std::numeric_limits<T>::epsilon());
}

template<typename T>
constexpr inline bool less_than_or_equal(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return less_than(lhs, rhs) || equal(lhs, rhs);
}

template<typename T>
constexpr inline bool greater_than_or_equal(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return greater_than(lhs, rhs) || equal(lhs, rhs);
}

template<typename T>
constexpr inline T abs_sin(T const & value)
{
    static_assert(std::is_floating_point_v<T>);
    return map(std::sin(value), -1.0, 1.0, 0.0, 1.0);
}

template<typename T>
constexpr inline T abs_cos(T const & value)
{
    static_assert(std::is_floating_point_v<T>);
    return map(std::cos(value), -1.0, 1.0, 0.0, 1.0);
}

template<typename T>
constexpr inline T abs_tan(T const & value)
{
    static_assert(std::is_floating_point_v<T>);
    return map(std::tan(value), -1.0, 1.0, 0.0, 1.0);
}

} // End of namespace
