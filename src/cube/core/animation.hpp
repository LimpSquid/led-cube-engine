#pragma once

#include <boost/noncopyable.hpp>

namespace cube::core
{

class graphics_device;
class animation :
    boost::noncopyable
{
public:
    virtual ~animation() = default;

    bool dirty() const;

    void init();
    void update();
    void finish();
    void paint_event(graphics_device & device);

protected:
    animation();

private:
    virtual void configure();
    virtual void paint(graphics_device & device) = 0;
    virtual void stop();

    bool dirty_;
};

}
