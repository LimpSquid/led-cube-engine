#pragma once

#include <cube/core/timers.hpp>

namespace cube::core
{

class graphics_device;
class animation
{
public:
    virtual ~animation() = default;

    engine_context & context();

    bool dirty() const;

    void init();
    void update();
    void finish();
    void paint_event(graphics_device & device);

protected:
    animation(engine_context & context);

private:
    animation(animation &) = delete;
    animation(animation &&) = delete;

    virtual void start();
    virtual void scene_tick(std::chrono::milliseconds dt);
    virtual void paint(graphics_device & device) = 0;
    virtual void stop();

    engine_context & context_;
    recurring_timer scene_timer_;
    bool dirty_;
};

}
