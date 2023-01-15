#pragma once

#include <cube/core/timers.hpp>

namespace cube::core
{

class graphics_device;
class animation
{
public:
    enum animation_state {
        stopped,
        running,
        stopping
    };

    virtual ~animation() = default;

    engine_context & context();

    animation_state state() const;
    bool dirty() const;

    void init();
    void update();
    void about_to_finish();
    void finish();
    void paint_event(graphics_device & device);

protected:
    animation(engine_context & context);

    void change_state(animation_state state);

private:
    animation(animation &) = delete;
    animation(animation &&) = delete;

    virtual void scene_tick(std::chrono::milliseconds dt);
    virtual void paint(graphics_device & device) = 0;
    virtual void state_changed(animation_state state);

    engine_context & context_;
    recurring_timer scene_timer_;
    animation_state state_;
    bool dirty_;
};

}
