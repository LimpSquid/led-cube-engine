#pragma once

#include <http/types.h>
#include <types/type_traits.h>
#include <string>

namespace rest::http
{

class streamable_body
{
public:
    streamable_body(response_type &response);
    ~streamable_body();

    streamable_body &operator<<(const std::string &string);
    streamable_body &operator<<(std::string &&string);

    template <class T>
    typename std::enable_if<rest::types::has_serialize<T>::value, streamable_body &>::type
    operator<<(const T &obj)
    {
        return operator<<(obj.serialize());
    }

    template <class T>
    typename std::enable_if<rest::types::has_serialize<T>::value, streamable_body &>::type
    operator<<(T &obj)
    {
        return operator<<(obj.serialize());
    }


    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value && rest::types::is_string_convertible<T>::value, streamable_body &>::type
    operator<<(T value)
    {
        return operator<<(std::to_string(value));
    }

private:
    body_type::value_type &body_;
};

}