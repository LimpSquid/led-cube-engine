#pragma once

#include <cube/core/timers.hpp>

namespace cube::gfx
{

struct transition_config
{
    double from;
    double to;
    unsigned int resolution;
    std::chrono::milliseconds time;
};

class basic_transition
{
public:
    basic_transition(core::engine_context & context, transition_config const & config);

    double value() const;
    void start();

private:
    virtual double map(double progress) const = 0; // map 0.0 - 1.0

    core::recurring_timer timer_;

    transition_config config_;
    double value_;
    unsigned int step_;
};

class linear_transition :
    public basic_transition
{
public:
    linear_transition(core::engine_context & context, transition_config const & config);

private:
    double map(double progress) const;
};

class sine_transition :
    public basic_transition
{
public:
    sine_transition(core::engine_context & context, transition_config const & config);

private:
    double map(double progress) const;
};

}
