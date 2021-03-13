#include <router.h>
#include <routernode.h>
#include <stdexcept>
#include <utility>

using namespace Rest;

Router::Router()
{

}

Router::~Router()
{

}

RouterNode &Router::makeNode(const std::string &expression)
{
    RouterNode newNode(expression);

    for(const RouterNode &node : nodes_) {
        if(node == newNode)
            throw std::invalid_argument("Expression already specified for router");
    }
    return nodes_.emplace_back(std::move(newNode));
}