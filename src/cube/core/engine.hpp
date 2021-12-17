#pragma once


#include <boost/noncopyable.hpp>
#include <memory>

namespace cube::core
{

class animation;
class engine_context;
class graphics_device;
class engine :
    boost::noncopyable
{
public:
    engine(engine_context & context, graphics_device * device);
    ~engine() = default;

    void load(animation * animation);
    void run();

    // Todo: eventually add ZMQ and ASIO stuff to the cube which external systems can use as well.

private:
    engine_context & context_;
    animation * animation_;
    std::unique_ptr<graphics_device> device_;
};

} // End of namespace
