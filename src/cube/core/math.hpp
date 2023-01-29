#pragma once

#include <boost/safe_numerics/safe_integer.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

namespace cube::core
{

template<typename T>
struct range
{
    constexpr range(T f, T t) :
        from(std::move(f)),
        to(std::move(t))
    { }

    T from;
    T to;
};

template<typename T>
struct safe_range
{
    constexpr safe_range(range<T> const & unsafe) :
        from(unsafe.from),
        to(unsafe.to)
    { }

    static_assert(std::is_integral_v<T>);
    boost::safe_numerics::safe<T> from;
    boost::safe_numerics::safe<T> to;
};

template<typename T>
constexpr range<T> make_unit_circle_range()
{
    static_assert(std::is_floating_point_v<T>);
    return {T{-1.0}, T{1.0}};
}

constexpr range rand_range{0, RAND_MAX};
constexpr range unit_circle_range = make_unit_circle_range<double>();
constexpr range randd_range{0.0, 1.0};
constexpr range randf_range{0.0f, 1.0f};

template<typename T>
constexpr range<T> make_limit_range()
{
    return range(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
}

template<typename TOut, typename TIn>
constexpr range<TOut> range_cast(range<TIn> const & in)
{
    return range(TOut(in.from), TOut(in.to));
}

template<typename T>
constexpr range<T> operator*(range<T> const & lhs, range<T> const & rhs) { return {lhs.from * rhs.from, lhs.to * rhs.to}; }
template<typename T>
constexpr range<T> operator*(range<T> const & lhs, double scalar) { return {lhs.from * scalar, lhs.to * scalar}; }

template<typename TIn, typename TOut>
constexpr TOut map(
    TIn const & value,
    range<TIn> const & in_range,
    range<TOut> const & out_range)
{
    auto const do_map = [](auto const & value, auto const & in_range, auto const & out_range) {
        return out_range.from + diff(out_range) * (value - in_range.from) / diff(in_range);
    };

    if constexpr(std::is_floating_point_v<TIn> && std::is_integral_v<TOut>)
        return static_cast<TOut>(std::round(do_map(value, in_range, range_cast<TIn>(out_range))));
    else if constexpr(std::is_integral_v<TIn> && std::is_floating_point_v<TOut>)
        return do_map(static_cast<TOut>(value), range_cast<TOut>(in_range), out_range);
    else if constexpr(std::is_integral_v<TIn> && std::is_integral_v<TOut>)
        return do_map(boost::safe_numerics::safe<TIn>(value), safe_range(in_range), safe_range(out_range));
    else
        return do_map(value, in_range, out_range);
}

template<typename TIn, typename TOut>
constexpr TOut map(
    TIn const & value,
    TIn const &  in_from, TIn const & in_to,
    TOut const & out_from, TOut const &  out_to)
{
    return map(value, range<TIn>(in_from, in_to), range(out_from, out_to));
}

template<typename T>
constexpr bool within_inclusive(T const & value, range<T> const & range)
{
    if constexpr (std::is_floating_point_v<T>)
        return greater_than_or_equal(value, range.from) && less_than_or_equal(value, range.to);
    else
        return value >= range.from && value <= range.to;
}

template<typename T>
constexpr bool within_exclusive(T const & value, range<T> const & range)
{
    if constexpr (std::is_floating_point_v<T>)
        return greater_than_or_equal(value, range.from) && less_than(value, range.to);
    else
        return value >= range.from && value < range.to;
}

template<typename T>
constexpr T clamp(T const & value, range<T> const & range)
{
    return std::clamp(value, range.from, range.to);
}

template<typename T>
constexpr T diff(range<T> const & range)
{
    return range.to - range.from;
}

template<typename T>
constexpr range<T> reverse(range<T> const & range)
{
    return {range.to, range.from};
}

template<typename T>
constexpr T diff(safe_range<T> const & range)
{
    return range.to - range.from;
}

template<typename T>
constexpr bool equal(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return std::abs(lhs - rhs) <= ((std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * std::numeric_limits<T>::epsilon());
}

template<typename T>
constexpr bool greater_than(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return (lhs - rhs) > ((std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * std::numeric_limits<T>::epsilon());
}

template<typename T>
constexpr bool less_than(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return (rhs - lhs) > ((std::abs(lhs) < std::abs(rhs) ? std::abs(rhs) : std::abs(lhs)) * std::numeric_limits<T>::epsilon());
}

template<typename T>
constexpr bool less_than_or_equal(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return less_than(lhs, rhs) || equal(lhs, rhs);
}

template<typename T>
constexpr bool greater_than_or_equal(T lhs, T rhs)
{
    static_assert(std::is_floating_point_v<T>);
    return greater_than(lhs, rhs) || equal(lhs, rhs);
}

template<typename T>
constexpr T map_sin(T const & value, range<T> range = range(T{0}, T{1}))
{
    static_assert(std::is_floating_point_v<T>);
    return map(std::sin(value), make_unit_circle_range<T>(), range);
}

template<typename T>
constexpr T map_cos(T const & value, range<T> range = range(T{0}, T{1}))
{
    static_assert(std::is_floating_point_v<T>);
    return map(std::cos(value), make_unit_circle_range<T>(), range);
}

inline double randd(range<double> range = randd_range) // Inclusive
{
    return map(rand(), rand_range, range);
}

inline float randf(range<float> range = randf_range) // Inclusive
{
    return map(rand(), rand_range, range);
}

template<typename T = int>
T rand()
{
    static_assert(std::numeric_limits<T>::max() >= RAND_MAX);
    return static_cast<T>(std::rand());
}

template<typename T>
T rand(range<T> range) // Inclusive
{
    static_assert(std::is_integral_v<T>);
    return map(rand<double>(), range_cast<double>(rand_range), range); // Use double so output is rounded and we avoid integer overflows
}

template<typename T, typename F>
void smooth(T & accumulator, T const & sample, F factor)
{
    accumulator = (sample / factor) + accumulator - (accumulator / factor);
}

} // End of namespace
