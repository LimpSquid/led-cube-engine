#pragma once

#include <cube/core/math.hpp>
#include <cube/core/timers.hpp>

namespace cube::gfx
{

using easing_range_t = core::range<double>;

struct easing_config
{
    easing_range_t range;
    unsigned int resolution;
    std::chrono::milliseconds time;
};

template<typename EasingCurve, bool Inverse = false>
class basic_easing_transition
{
public:
    using completion_handler_t = std::function<void()>;

    basic_easing_transition(core::engine_context & context, easing_config config,
        std::optional<completion_handler_t> completion_handler = {}) :
        timer_(context, std::bind(&basic_easing_transition::operator(), this)),
        completion_handler_(std::move(completion_handler)),
        config_(std::move(config)),
        value_(config_.range.from)
    {
        if (config_.resolution == 0)
            throw std::invalid_argument("Parameter easing_config::resolution cannot be zero");
    }

    double value() const { return value_; }
    void stop() { timer_.stop(); }
    void start()
    {
        step_ = 0;
        value_ = config_.range.from;
        timer_.start(config_.time / config_.resolution);
    }

private:
    using easing_curve_t = EasingCurve;
    using inverse = std::integral_constant<bool, Inverse>;

    void operator()()
    {
        constexpr easing_range_t curve_range{0.0, 1.0};
        static const easing_curve_t curve{};

        if (++step_ < config_.resolution) {
            double curve_position;
            double const position = static_cast<double>(step_) / config_.resolution;
            if constexpr(inverse::value)
                curve_position = 1.0 - core::clamp(curve(1.0 - position), curve_range);
            else
                curve_position = core::clamp(curve(position), curve_range);
            value_ = config_.range.from + diff(config_.range) * curve_position;
        } else {
            value_ = config_.range.to;
            timer_.stop();
            if (completion_handler_)
                (*completion_handler_)();
        }
    }

    core::recurring_timer timer_;
    std::optional<completion_handler_t> completion_handler_;
    easing_config const config_;
    double value_;
    unsigned int step_;
};

struct linear_curve { double operator()(double) const; };
using ease_linear = basic_easing_transition<linear_curve>;

struct sine_curve { double operator()(double) const; };
using ease_in_sine = basic_easing_transition<sine_curve>;
using ease_out_sine = basic_easing_transition<sine_curve, true>;

struct bounce_curve { double operator()(double) const; };
using ease_in_bounce = basic_easing_transition<bounce_curve>;
using ease_out_bounce = basic_easing_transition<bounce_curve, true>;

}
