#ifdef EVAL_EXPRESSION
#pragma once

#include <type_traits>
#include <string>

namespace cube::core
{

float evalf(std::string const & str);
double evald(std::string const & str);
long double evalld(std::string const & str);

template<typename T>
typename std::enable_if_t<std::is_floating_point_v<T>, T> eval(std::string const & str)
{
    if constexpr (std::is_same_v<T, float>)
        return evalf(str);
    if constexpr (std::is_same_v<T, double>)
        return evald(str);
    if constexpr (std::is_same_v<T, long double>)
        return evalld(str);
}

} // End of namespace
#endif
