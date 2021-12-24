#pragma once

#include <boost/safe_numerics/safe_integer.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

namespace cube::core
{

template<typename T>
struct Range
{
    constexpr Range(T const & f, T const & t) :
        from(f),
        to(t)
    {
        if (from == to)
            throw std::runtime_error("from == to");
    }

    T from;
    T to;
};

template<typename T>
struct SafeRange
{
    constexpr SafeRange(Range<T> const & unsafe) :
        from(unsafe.from),
        to(unsafe.to)
    { }

    static_assert(std::is_integral_v<T>);
    boost::safe_numerics::safe<T> from;
    boost::safe_numerics::safe<T> to;
};

constexpr Range unit_circle_range = {-1.0, 1.0};

template<typename T>
constexpr inline Range<T> make_limit_range()
{
    return Range(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
}

template<typename TOut, typename TIn>
constexpr inline Range<TOut> range_cast(Range<TIn> const & in)
{
    return Range<TOut>(in.from, in.to);
}

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
    auto do_map = [](auto const & value, auto const & in_range, auto const & out_range) {
        return out_range.from + (out_range.to - out_range.from) * (value - in_range.from) / (in_range.to - in_range.from);
    };

    if constexpr(std::is_floating_point_v<TIn> && std::is_integral_v<TOut>)
        return std::round(do_map(value, in_range, range_cast<TIn>(out_range)));
    if constexpr(std::is_integral_v<TIn> && std::is_floating_point_v<TOut>)
        return do_map(value, range_cast<TOut>(in_range), out_range);
    else if constexpr(std::is_integral_v<TIn> && std::is_integral_v<TOut>)
        return do_map(boost::safe_numerics::safe<TIn>(value), SafeRange(in_range), SafeRange(out_range));
    else
        return do_map(value, in_range, out_range);
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
    return map(std::sin(value), unit_circle_range, Range(0.0, 1.0));
}

template<typename T>
constexpr inline T abs_cos(T const & value)
{
    static_assert(std::is_floating_point_v<T>);
    return map(std::cos(value), unit_circle_range, Range(0.0, 1.0));
}

} // End of namespace
