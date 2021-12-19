#include <cube/gfx/transition.hpp>
#include <cube/core/math.hpp>

using namespace cube::core;

namespace cube::gfx
{

basic_transition::basic_transition(core::engine_context & context, transition_config const & config) :
    context_(context),
    config_(config),
    value_(config_.from)
{
    if (config_.steps == 0)
        throw std::runtime_error("transition_config::steps cannot be zero");
}

double basic_transition::value() const
{
    return value_;
}

void basic_transition::start()
{
    step_ = 0;
    value_ = config_.from;

    tick_sub_ = tick_subscription::create(
        context_,
        config_.time / config_.steps,
        [this](auto, auto) {
            if (++step_ < config_.steps) {
                double progress = std::clamp(map(static_cast<double>(step_) / config_.steps), 0.0, 1.0);
                value_ = config_.from + (config_.to - config_.from) * progress;
            } else {
                value_ = config_.to;
                tick_sub_.reset();
            }
        }
    );
}

linear_transition::linear_transition(engine_context & context, transition_config const & config) :
    basic_transition(context, config)
{ }

double linear_transition::map(double progress) const
{
    return progress;
}

sine_transition::sine_transition(engine_context & context, transition_config const & config) :
    basic_transition(context, config)
{ }

double sine_transition::map(double progress) const
{
    return std::sin(0.5 * M_PI * progress);
}

}
