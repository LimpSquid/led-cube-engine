#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <type_traits>
#include <stdexcept>
#include <boost/noncopyable.hpp>

namespace cube::gfx
{

template<typename PropertyLabelType>
class basic_animation_track : private boost::noncopyable
{
public:
    using property_label_type = PropertyLabelType;

    basic_animation_track() = default;
    ~basic_animation_track() = default;

    template<typename T>
    typename std::enable_if<std::is_string_convertible<T>::value>::type write_property(const property_label_type &label, T value)
    {
        properties_[label] = std::to_string(value);
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value ||
        std::is_floating_point<T>::value, T>::type read_property(const property_label_type &label, T def = T())
    {
        const auto search = properties_.find(label);

        if(search == properties_.end() || search->second.empty())
            return def;

        T result;
        std::stringstream stream(search->second);

        stream >> result;
        return result;
    }

private:
    template<class, class = void>
    struct is_string_convertible : public std::false_type { };

    template<class T>
    struct is_string_convertible<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> : public std::true_type { };

    std::unordered_map<property_label_type, std::string> properties_;
};

using animation_track = basic_animation_track<std::string>;

}