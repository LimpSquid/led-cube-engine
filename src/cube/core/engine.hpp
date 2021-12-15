#pragma once

#include <cube/core/animation.hpp>
#include <memory>
#include <boost/noncopyable.hpp>

namespace cube::core
{

class graphics_device;
class engine :
    private boost::noncopyable
{
public:
    engine(graphics_device * device);
    ~engine() = default;

    void load(animation * animation);
    void run();

    // Todo: eventually add ZMQ and ASIO stuff to the cube which external systems can use as well.

private:
    animation * animation_;
    std::unique_ptr<graphics_device> device_;
};

} // end of namespace
