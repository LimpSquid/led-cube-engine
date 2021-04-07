#pragma once

#include <chrono>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

namespace cube::core
{

struct animation_config
{
    std::chrono::milliseconds time_step_ms = std::chrono::milliseconds(50);
};

class graphics_device;
class animation : public boost::enable_shared_from_this<animation>,
    private boost::noncopyable
{
public:
    using pointer = boost::shared_ptr<animation>;

    virtual ~animation() = default;

    const animation_config &config() const;
    bool dirty() const;

    void init();
    void update();

    void time_step_event();
    void tick_event(const std::chrono::microseconds &interval);
    void paint_event(graphics_device &device);

protected:
    animation();

    virtual void configure(animation_config &config);
    virtual void time_step();
    virtual void tick(const std::chrono::microseconds &interval) = 0;
    virtual void paint(graphics_device &device) = 0;

private:
    animation_config config_;
    bool dirty_;
};

}