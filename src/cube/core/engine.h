#pragma once

#include <boost/noncopyable.hpp>

namespace cube::core
{

class engine : private boost::noncopyable
{
public:
    engine() = default;
    ~engine() = default;
};

}