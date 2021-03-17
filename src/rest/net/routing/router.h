#pragma once

#include <net/routing/router_node.h>
#include <net/routing/token_data.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace rest::net::routing
{

class router : private boost::noncopyable
{
public:
    using pointer = boost::shared_ptr<router>;

    static pointer create()
    {
        return pointer(new router);
    }

    ~router() = default;

    template<class RouterHandlerImpl, class ...RouterHandlerArgs>
    RouterHandlerImpl &make_handler(const std::string &url_expression, const RouterHandlerArgs &...args)
    {
        router_handler::pointer handler = router_handler::create<RouterHandlerImpl>(args...);
        router_node new_node(url_expression, handler);

        for(const router_node &node : nodes_) {
            if(node == new_node)
                throw std::invalid_argument("Url expression already specified for router");
        }

        nodes_.emplace_back(std::move(new_node));
        return *static_cast<RouterHandlerImpl *>(handler.get());
    }


    template<class RouterHandlerImpl, class ...RouterHandlerArgs>
    bool handle(const std::string &url, RouterHandlerArgs &...args)
    {
        if(url.empty())
            return false;

        token_data data; // @Todo: pass along to handle method...
        for(router_node &node : nodes_) {
            if(node.match(url, data)) {
                node.handler().handle<RouterHandlerImpl>(args...);
                return true;
            }
            data.clear();
        }

        return false;
    }

private:
    router() = default;

    std::vector<router_node> nodes_;
};

}