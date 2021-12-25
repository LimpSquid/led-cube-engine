#pragma once

#include <cube/core/timers.hpp>
#include <boost/noncopyable.hpp>

namespace cube::core
{

class graphics_device;
class animation :
    boost::noncopyable
{
public:
    virtual ~animation() = default;

    engine_context & context();

    char const * const name() const;
    bool dirty() const;

    void init();
    void update();
    void finish();
    void paint_event(graphics_device & device);

protected:
    animation(engine_context & context, char const * const name);

private:
    virtual void start();
    virtual void paint(graphics_device & device) = 0;
    virtual void stop();

    engine_context & context_;
    char const * const name_;
    bool dirty_;
};

using scene_update_handler_t = std::function<void()>;

class animation_scene
{
public:
    animation_scene(animation & animation, std::optional<scene_update_handler_t> handler);

    void start();
    void stop();

private:
    recurring_timer timer_;
};

}
