#pragma once

#include <boost/preprocessor.hpp>
#include <type_traits>

#define ENUM_PROCESS_ONE(r, start_index, index, element) BOOST_PP_COMMA_IF(index) BOOST_PP_IF(index, element, element = start_index)
#define ENUM_PROCESS_ONE_STRING(r, unused, index, element) BOOST_PP_COMMA_IF(index) BOOST_PP_STRINGIZE(element)
#define ENUM(enum_name, enum_type, start_index, ...) \
    enum enum_name : enum_type { BOOST_PP_SEQ_FOR_EACH_I(ENUM_PROCESS_ONE, start_index, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) }; \
    friend char const * const to_string(enum_name p) \
    { \
        static const char * const strings[] = { BOOST_PP_SEQ_FOR_EACH_I(ENUM_PROCESS_ONE_STRING, %%, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) }; \
        static size_t size = sizeof(strings) / sizeof(strings[0]); \
        enum_type offset = p - start_index; \
        if constexpr (std::is_signed_v<enum_type>) return (offset < 0 || offset >= size) ? "???" : strings[offset]; \
        else return (offset >= size) ? "???" : strings[offset]; \
    }
