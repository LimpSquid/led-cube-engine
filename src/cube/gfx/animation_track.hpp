#pragma once

#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace cube::gfx
{

template<typename T>
struct property_value_converter
{
    static std::string convert(T const & value)
    {
        std::stringstream stream;
        stream << value;
        return stream.str();
    }

    static T convert(std::string const & value)
    {
        T result;
        std::stringstream stream(value);
        stream >> result;
        return result;
    }
};

template<>
struct property_value_converter<std::string>
{
    static std::string convert(std::string const & value) { return value; }
};

template<class Rep, class Period>
struct property_value_converter<std::chrono::duration<Rep, Period>>
{
    static std::string convert(std::chrono::duration<Rep, Period> const & value)
    {
        return property_value_converter<Rep>::convert(value.count());
    }

    static std::chrono::duration<Rep, Period> convert(std::string const & value)
    {
        auto const count = property_value_converter<Rep>::convert(value);
        return std::chrono::duration<Rep, Period>(count);
    }
};

template<>
struct property_value_converter<core::color>
{
    static std::string convert(core::color const & value)
    {
        std::stringstream stream;
        stream << value.r << value.g << value.b << value.a;
        return stream.str();
    }

    static core::color convert(std::string const & value)
    {
        core::color_t r, g, b, a;
        std::stringstream stream(value);
        stream >> r >> g >> b >> a;
        return {r, g, b, a};
    }
};

struct property_value
{
    template<typename T>
    property_value(T const & v) :
        property(property_value_converter<T>::convert(v))
    { }

    std::string const property;
};

template<>
struct property_value_converter<property_value>
{
    static std::string convert(property_value const & value) { return value.property; }
};

class animation_track :
    public core::animation
{
public:
    using property_label_type = int;

    enum animation_state
    {
        stopped = 0,
        paused,
        running,
        finished,
    };

    enum : property_label_type
    {
        animation_time_us   = 0,

        property_custom     = 255, // First usable label for custom properties
    };

    animation_state state() const;
    bool is_stopped() const;
    bool is_paused() const;
    bool is_running() const;
    bool is_finished() const;

    std::chrono::microseconds time_remaining() const;
    std::chrono::microseconds time_total() const;

    void start();
    void pause();
    void stop();

    template<typename T>
    void write_property(property_label_type label, T value)
    {
        properties_[label] = property_value_converter<T>::convert(value);
    }

    void write_properties(std::vector<std::pair<property_label_type, property_value>> const & properties);

    template<typename T>
    T read_property(property_label_type label, T def = T()) const
    {
        auto const search = properties_.find(label);

        if (search == properties_.end() || search->second.empty())
            return def;

        // Todo: maybe do some checking if we are able to convert the string to the given type.
        return property_value_converter<T>::convert(search->second);
    }

protected:
    animation_track();

private:
    virtual void state_changed(animation_state const & state);
    virtual void tick(std::chrono::microseconds const & interval) override;

    void set_state(animation_state const & value);

    animation_state state_;
    std::chrono::microseconds time_;
    std::unordered_map<property_label_type, std::string> properties_;
};

template<class, class = void>
struct is_animation_track : std::false_type { };
template<class T>
struct is_animation_track<T, std::void_t<decltype(
    std::declval<T>().start(),
    std::declval<T>().stop(),
    std::declval<T>().pause(),
    std::declval<T>().is_stopped(),
    std::declval<T>().is_running(),
    std::declval<T>().is_paused(),
    std::declval<T>().is_finished()
)>> : std::true_type { };

} // end of namespace
