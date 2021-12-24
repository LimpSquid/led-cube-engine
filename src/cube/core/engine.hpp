#pragma once


#include <boost/noncopyable.hpp>
#include <memory>

namespace cube::core
{

class animation;
class engine_context;
class graphics_device;

template<typename T>
struct graphics_device_factory
{
    using type = T;

    std::unique_ptr<type> operator()(engine_context & context) const
    {
        return std::make_unique<type>(context);
    }
};

class engine :
    boost::noncopyable
{
public:
    template<typename T>
    engine(engine_context & context, graphics_device_factory<T> factory) :
        context_(context),
        device_(factory(context)),
        animation_(nullptr)
    { }

    void load(animation * animation);
    void run();

    // Todo: eventually add ZMQ and ASIO stuff to the cube which external systems can use as well.

private:
    engine_context & context_;
    animation * animation_;
    std::unique_ptr<graphics_device> device_;
};

} // End of namespace
