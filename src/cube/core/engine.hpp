#pragma once


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

class engine
{
public:
    template<typename T>
    engine(engine_context & context, graphics_device_factory<T> factory) :
        context_(context),
        animation_(nullptr),
        device_(factory(context))
    { }

    engine_context & context();
    void load(animation * animation); // Todo: make shared pointer, so lifetime can be managed properly
    void run();

private:
    engine(engine & other) = delete;
    engine(engine && other) = delete;

    engine_context & context_;
    animation * animation_;
    std::unique_ptr<graphics_device> device_;
};

} // End of namespace
