#pragma once

#include <type_traits>
#include <string>

namespace rest::types
{

template<class, class = void>
struct __is_string_convertible_impl : public std::false_type { };

template<class T>
struct __is_string_convertible_impl<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> : public std::true_type { };

template<class, class, class, class ...>
struct __has_handle_impl : public std::false_type { };

template<class Handler, class ResourceData, class ...HandlerArgs>
struct __has_handle_impl<std::void_t<decltype(std::declval<Handler>().handle(std::declval<ResourceData>(), std::declval<HandlerArgs>()...))>,
    Handler, ResourceData, HandlerArgs...> : public std::true_type { };

template<class T>
using is_string_convertible = __is_string_convertible_impl<T>;

template<class Handler, class ResourceData, class ...HandlerArgs>
using has_handle = __has_handle_impl<std::void_t<>, Handler, ResourceData, HandlerArgs...>;

}