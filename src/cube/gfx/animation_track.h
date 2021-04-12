#pragma once

#include <core/animation.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <type_traits>
#include <stdexcept>

namespace cube::gfx
{

template<typename PropertyLabelType>
class basic_animation_track : public core::animation
{
private:
    template<class, class = void>
    struct is_string_convertible : public std::false_type { };

    template<class T>
    struct is_string_convertible<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> : public std::true_type { };

public:
    using property_label_type = PropertyLabelType;

    enum animation_state
    {
        stopped = 0,
        paused,
        running,
        finished,
    };

    ~basic_animation_track() = default;

    void start()
    {
        switch(state_) {
            case stopped:
                duration_ = duration_us();
                [[fallthrough]];
            case paused:
                set_state(running);
                break;
            default:;
        }
    }

    void pause()
    {
        switch(state_) {
            case running:
                set_state(paused);
                break;
            default:;
        }
    }

    void stop()
    {
        switch(state_) {
            case paused:
            case running:
                set_state(stopped);
                break;
            default:;
        }
    }

    animation_state poll() const
    {
        if(poll_at_end())
            set_state(finished);
        else if(poll_duration_expired())
            set_state(finished);

        return state_;
    }

    template<typename T>
    typename std::enable_if<is_string_convertible<T>::value>::type write_property(const property_label_type &label, T value)
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

protected:
    basic_animation_track() : state_(stopped) { }

    std::chrono::microseconds us_to_end() const { return duration_; }
    virtual std::chrono::microseconds duration_us() const { return std::chrono::microseconds::max(); }
    virtual bool poll_at_end() const { return false; }
    virtual void state_changed(const animation_state &) { }

private:
    virtual void tick(const std::chrono::microseconds &interval)
    {
        using namespace std::chrono_literals;

        if(running == state_)
            duration_ = interval < duration_ ? (duration_ - interval) : 0s;
    }

    bool poll_duration_expired() const
    {
        return running == state_ && duration_.count() <= 0;
    }

    void set_state(const animation_state &value)
    {
        if(state_ != value) {
            state_ = value;
            state_changed(state_);
        }
    }

    std::unordered_map<property_label_type, std::string> properties_;
    animation_state state_;
    std::chrono::microseconds duration_;
};

using animation_track = basic_animation_track<std::string>;

}