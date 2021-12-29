#pragma once

#include <boost/preprocessor.hpp>
#include <type_traits>
#include <array>

#define ENUM_PROCESS_ONE(r, start_index, index, element) BOOST_PP_COMMA_IF(index) BOOST_PP_IF(index, element, element = start_index)
#define ENUM_PROCESS_ONE_STRING(r, unused, index, element) BOOST_PP_COMMA_IF(index) BOOST_PP_STRINGIZE(element)
#define ENUM_TO_STRING_IMPL(enum_name, enum_type, start_index, ...) \
    char const * const to_string(enum_name p) \
    { \
        constexpr std::size_t size = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__); \
        static const std::array<char const *, size> strings = \
            { BOOST_PP_SEQ_FOR_EACH_I(ENUM_PROCESS_ONE_STRING, %%, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) }; \
        enum_type offset = static_cast<enum_type>(p) - start_index; \
        if constexpr (std::is_signed_v<enum_type>) return (offset < 0 || offset >= size) ? "???" : strings[offset]; \
        else return (offset >= size) ? "???" : strings[offset]; \
    }
#define ENUM(enum_name, enum_type, start_index, ...) \
    enum enum_name : enum_type { BOOST_PP_SEQ_FOR_EACH_I(ENUM_PROCESS_ONE, start_index, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) }; \
    friend ENUM_TO_STRING_IMPL(enum_name, enum_type, start_index, __VA_ARGS__)
#define NS_ENUM(enum_name, enum_type, start_index, ...) \
    enum class enum_name : enum_type { BOOST_PP_SEQ_FOR_EACH_I(ENUM_PROCESS_ONE, start_index, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) }; \
    inline ENUM_TO_STRING_IMPL(enum_name, enum_type, start_index, __VA_ARGS__)
