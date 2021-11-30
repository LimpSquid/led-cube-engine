#pragma once

#include <cube/core/animation.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <type_traits>

namespace cube::gfx
{

template<typename T>
struct property_value_converter
{
    static std::string convert(T const & value)
    {
        std::string str;
        std::stringstream stream(str);
        stream << value;
        return str;
    }

    static T convert(std::string const & value)
    {
        T result;
        std::stringstream stream(value);
        stream >> result;
        return result;
    }
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
        property_duration_us    = 0,

        // Reserved for user properties
        property_user           = 255,
    };

    virtual ~animation_track() override = default;

    animation_state state() const;
    bool is_stopped() const;
    bool is_paused() const;
    bool is_running() const;
    bool is_finished() const;

    std::chrono::microseconds us_to_end() const;
    std::chrono::microseconds duration_us() const;

    void start();
    void pause();
    void stop();

    template<typename T>
    void write_property(property_label_type label, T value)
    {
        properties_[label] = property_value_converter<T>::convert(value);
    }

    template<typename T>
    T read_property(property_label_type label, T def = T()) const
    {
        auto const search = properties_.find(label);

        if (search == properties_.end() || search->second.empty())
            return def;

        return property_value_converter<T>::convert(search->second);
    }

protected:
    animation_track();

private:
    virtual void state_changed(animation_state const & state);
    virtual void tick(std::chrono::microseconds const & interval) override;

    void set_state(animation_state const & value);

    animation_state state_;
    std::chrono::microseconds duration_;
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

}
