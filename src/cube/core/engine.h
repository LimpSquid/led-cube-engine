#pragma once

#include <core/animation.h>
#include <memory>
#include <thread>
#include <boost/noncopyable.hpp>

namespace cube::core
{

class graphics_device;
class engine : private boost::noncopyable
{
public:
    engine(graphics_device *device);
    ~engine();

    void load(const animation::pointer &animation);

private:
    void process();

    animation::pointer animation_;
    std::unique_ptr<graphics_device> device_;
    std::thread thread_;
};

}