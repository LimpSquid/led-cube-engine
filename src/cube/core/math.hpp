#pragma once

// pull in some math libraries from cpp standard
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace cube::core
{

template<typename TIn, typename TOut>
inline TOut map(
    TIn const & value,
    TIn const &  in_from, TIn const & in_to,
    TOut const & out_from, TOut const &  out_to)
{
    return out_from + (out_to - out_from) * (value - in_from) / (in_to - in_from);
}

inline bool equal(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) <= ( (std::fabs(lhs) < std::fabs(rhs) ? std::fabs(rhs) : std::fabs(lhs)) * DBL_EPSILON);
}

inline bool equal(float lhs, float rhs)
{
    return std::fabs(lhs - rhs) <= ( (std::fabs(lhs) < std::fabs(rhs) ? std::fabs(rhs) : std::fabs(lhs)) * FLT_EPSILON);
}

inline bool greater_than(double lhs, double rhs)
{
    return (lhs - rhs) > ( (std::fabs(lhs) < std::fabs(rhs) ? std::fabs(rhs) : std::fabs(lhs)) * DBL_EPSILON);
}

inline bool greater_than(float lhs, float rhs)
{
    return (lhs - rhs) > ( (std::fabs(lhs) < std::fabs(rhs) ? std::fabs(rhs) : std::fabs(lhs)) * FLT_EPSILON);
}

inline bool less_than(double lhs, double rhs)
{
    return (rhs - lhs) > ( (std::fabs(lhs) < std::fabs(rhs) ? std::fabs(rhs) : std::fabs(lhs)) * DBL_EPSILON);
}

inline bool less_than(float lhs, float rhs)
{
    return (rhs - lhs) > ( (std::fabs(lhs) < std::fabs(rhs) ? std::fabs(rhs) : std::fabs(lhs)) * FLT_EPSILON);
}

} // End of namespace
