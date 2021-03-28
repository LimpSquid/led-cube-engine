#pragma once

#include <chrono>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

namespace cube::core
{

class graphics_device;
class animation : public boost::enable_shared_from_this<animation>,
    private boost::noncopyable
{
public:
    using pointer = boost::shared_ptr<animation>;

    virtual ~animation() = default;

    bool dirty() const;
    void update();

    void tick_event(const std::chrono::microseconds &interval);
    void paint_event(graphics_device &device);

protected:
    animation();

    virtual void tick(const std::chrono::microseconds &interval) = 0;
    virtual void paint(graphics_device &device) = 0;

private:
    bool dirty_;
};

}