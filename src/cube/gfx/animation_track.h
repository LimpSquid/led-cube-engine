#pragma once

#include <core/animation.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <type_traits>
#include <stdexcept>

namespace cube::gfx
{

template<typename PropertyLabel>
class basic_animation_track : public core::animation
{
private:
template<class>
struct is_duration : std::false_type { };

template<class Rep, class Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type { };

public:
    using property_label_type = PropertyLabel;
    using pointer = boost::shared_ptr<basic_animation_track>;

    enum animation_state
    {
        stopped = 0,
        paused,
        running,
        finished,
    };

    virtual ~basic_animation_track() override = default;

    bool is_stopped() { return stopped == state_; }
    bool is_paused() { return paused == state_; }
    bool is_running() { return running == state_; }
    bool is_finished() { return finished == state_; }

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

    void poll() const
    {
        if(poll_at_end())
            set_state(finished);
        else if(poll_duration_expired())
            set_state(finished);
    }

    template<typename T>
    typename std::enable_if<is_duration<T>::value>::type write_property(const property_label_type &label, T value)
    {
        write_property(label, value.count());
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value ||
        std::is_floating_point<T>::value>::type write_property(const property_label_type &label, T value)
    {
        properties_[label] = std::to_string(value);
    }

    template<typename T>
    typename std::enable_if<is_duration<T>::value, T>::type read_property(const property_label_type &label, T def = T()) const
    {
        return T(read_property<int64_t>(label, def.count()));
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value ||
        std::is_floating_point<T>::value, T>::type read_property(const property_label_type &label, T def = T()) const
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
            duration_ = interval <= duration_ ? (duration_ - interval) : 0us;
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

    animation_state state_;
    std::chrono::microseconds duration_;
    std::unordered_map<property_label_type, std::string> properties_;
};

class property_animation_track : public basic_animation_track<int>
{
private:

public:
    enum : property_label_type
    {
        property_duration_us    = 0,

        // Reserved for user properties
        property_user           = 255,
    };

private:
    virtual std::chrono::microseconds duration_us() const override
    {
        return read_property<std::chrono::microseconds>(property_duration_us, basic_animation_track::duration_us());
    }
};

}