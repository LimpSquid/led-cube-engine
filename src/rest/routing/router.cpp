#include <routing/router.h>
#include <routing/router_node.h>
#include <stdexcept>
#include <utility>

using namespace rest::routing;

router::router()
{

}

router::~router()
{

}

router_node &router::make_node(const std::string &expression)
{
    router_node newNode(expression);

    for(const router_node &node : nodes_) {
        if(node == newNode)
            throw std::invalid_argument("Expression already specified for router");
    }
    return nodes_.emplace_back(std::move(newNode));
}