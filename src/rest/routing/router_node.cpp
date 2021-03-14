#include <routing/router_node.h>
#include <routing/expression_fragment.h>
#include <functional>

using namespace rest::routing;

router_node::router_node(const std::string &expression) :
    expr_hash_(std::hash<std::string>{ }(expression))
{

}

router_node::router_node(router_node &&other) :
    expr_hash_(other.expr_hash_)
{

}

router_node::~router_node()
{

}

bool router_node::match(const std::string &endpoint) const
{
    return false;
}

bool router_node::operator==(const router_node &other) const
{
    return (expr_hash_ == other.expr_hash_);
}

bool router_node::operator!=(const router_node &other) const
{
    return !operator==(other);
}