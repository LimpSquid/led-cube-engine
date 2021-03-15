#pragma once

#include <stdexcept>
#include <boost/shared_ptr.hpp>

namespace rest::net::routing
{

class router_handler
{
public:
    using pointer = boost::shared_ptr<router_handler>;

    template <class Handler, class... Args>
    static pointer create(const Args&... args) 
    { 
        return pointer(new Handler(args...));
    }

    template <class Handler, class... Args>
    void handle(Args&... args) 
    { 
        Handler *handler = dynamic_cast<Handler *>(this);

        if(nullptr == handler)
            throw std::invalid_argument("Invalid handler type");
        handler->handle(args...); 
    }
    
    virtual ~router_handler();

protected:
    router_handler();

};

}