#pragma once

#include <types/type_traits.h>
#include <net/routing/router_node.h>
#include <net/routing/routing_params.h>
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
    RouterHandlerImpl &make_handler(const std::string &resource_expression, const RouterHandlerArgs &...args)
    {
        static_assert(std::is_constructible<RouterHandlerImpl, const RouterHandlerArgs &...>::value, "RouterHandlerImpl is not constructible");

        router_handler::pointer handler = router_handler::create<RouterHandlerImpl>(args...);
        router_node new_node(resource_expression, handler);

        for(const router_node &node : nodes_) {
            if(node == new_node)
                throw std::invalid_argument("Resource expression already specified for router");
        }

        nodes_.emplace_back(std::move(new_node));
        return *static_cast<RouterHandlerImpl *>(handler.get());
    }


    template<class RouterHandlerImpl, class ...RouterHandlerArgs>
    bool handle(const std::string &resource, RouterHandlerArgs &...args)
    {
        static_assert(rest::types::has_handle<RouterHandlerImpl, routing_params, RouterHandlerArgs &...>::value,
                        "RouterHandlerImpl does not implement handle(...)");

        if(resource.empty())
            return false;

        routing_params params;
        for(router_node &node : nodes_) {
            if(node.match(resource, params)) {
                node.handler().handle<RouterHandlerImpl>(params, args...);
                return true;
            }
        }

        return false;
    }

private:
    router() = default;

    std::vector<router_node> nodes_;
};

}