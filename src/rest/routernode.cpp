#include <routernode.h>
#include <functional>

using namespace Rest;

RouterNode::RouterNode(const std::string &expression) :
    hash_(std::hash<std::string>{ }(expression))
{

}

RouterNode::RouterNode(RouterNode &&other) :
    hash_(other.hash_)
{

}

RouterNode::~RouterNode()
{

}

bool RouterNode::match(const std::string &endpoint) const
{
    return false;
}

bool RouterNode::operator==(const RouterNode &other) const
{
    return (hash_ == other.hash_);
}

bool RouterNode::operator!=(const RouterNode &other) const
{
    return !operator==(other);
}