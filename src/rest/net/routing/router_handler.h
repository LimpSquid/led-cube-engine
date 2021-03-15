#pragma once

#include <stdexcept>
#include <boost/shared_ptr.hpp>

namespace rest::net::routing
{

class router_handler
{
public:
    using pointer = boost::shared_ptr<router_handler>;

    template <class HandlerImpl, class ...HanderArgs>
    static pointer create(const HanderArgs &...args)
    { 
        return pointer(new HandlerImpl(args...));
    }

    template <class HandlerImpl, class ...HanderArgs>
    void handle(HanderArgs &...args)
    { 
        HandlerImpl *handler = dynamic_cast<HandlerImpl *>(this);

        if(nullptr == handler)
            throw std::invalid_argument("Invalid handler type");
        handler->handle(args...); 
    }
    
    virtual ~router_handler();

protected:
    router_handler();

};

}