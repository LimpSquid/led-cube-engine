#pragma once

#include <type_traits>
#include <string>

namespace rest::types
{

template<class, class = void>
struct __has_serialize_impl : public std::false_type { };

template<class T>
struct __has_serialize_impl<T, std::void_t<decltype(std::declval<T>().serialize())>> : public std::true_type { };

template<class, class = void>
struct __is_string_convertible_impl : public std::false_type { };

template<class T>
struct __is_string_convertible_impl<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> : public std::true_type { };

template<class, class, class ...>
struct __has_handle_impl : public std::false_type { };

template<class T, class ...Args>
struct __has_handle_impl<std::void_t<decltype(std::declval<T>().handle(std::declval<Args>()...))>,
    T, Args...> : public std::true_type { };

template<class T>
using has_serialize = __has_serialize_impl<T>;

template<class T>
using is_string_convertible = __is_string_convertible_impl<T>;

template<class T, class ...Args>
using has_handle = __has_handle_impl<std::void_t<>, T, Args...>;

}