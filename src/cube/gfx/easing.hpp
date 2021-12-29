#pragma once

#include <cube/core/timers.hpp>

namespace cube::gfx
{

struct easing_config
{
    double from;
    double to;
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
        timer_(context, [this, curve = EasingCurve{}, completion = std::move(completion_handler)](auto, auto) {
            if (++step_ < config_.resolution) {
                double progress;
                if constexpr(Inverse)
                    progress = 1.0 - std::clamp(curve(1.0 - static_cast<double>(step_) / config_.resolution), 0.0, 1.0);
                else
                    progress = std::clamp(curve(static_cast<double>(step_) / config_.resolution), 0.0, 1.0);
                value_ = config_.from + (config_.to - config_.from) * progress;
            } else {
                value_ = config_.to;
                timer_.stop();
                if (completion)
                    (*completion)();
            }
        }),
        config_(config),
        value_(config_.from)
    {
        if (config_.resolution == 0)
            throw std::invalid_argument("easing_config::resolution cannot be zero");
    }

    double value() const { return value_; }
    void stop() { timer_.stop(); }
    void start()
    {
        step_ = 0;
        value_ = config_.from;
        timer_.start(config_.time / config_.resolution);
    }

private:
    core::recurring_timer timer_;
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
