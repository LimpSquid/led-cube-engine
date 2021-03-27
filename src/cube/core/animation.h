#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

namespace cube::core
{

class graphics_device;
class time_step;
class animation : public boost::enable_shared_from_this<animation>,
    private boost::noncopyable
{
public:
    using pointer = boost::shared_ptr<animation>;

    virtual ~animation() = default;

    virtual void update(const time_step &time_step) = 0;
    virtual void paint(graphics_device &device) = 0;

protected:
    animation() = default;
};

}