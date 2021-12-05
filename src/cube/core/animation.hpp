#pragma once

#include <chrono>
#include <boost/noncopyable.hpp>

namespace cube::core
{

struct animation_config
{
    std::chrono::milliseconds time_step_interval{50};
};

class graphics_device;
class animation :
    private boost::noncopyable
{
public:
    virtual ~animation() = default;

    animation_config const & config() const;
    bool dirty() const;

    void init();
    void update();

    void time_step_event();
    void tick_event(std::chrono::microseconds const & interval);
    void paint_event(graphics_device & device);

protected:
    animation();

private:
    virtual void configure(animation_config & config);
    virtual void time_step();
    virtual void tick(std::chrono::microseconds const & interval) = 0;
    virtual void paint(graphics_device & device) = 0;

    animation_config config_;
    bool dirty_;
};

}
