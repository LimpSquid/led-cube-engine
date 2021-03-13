#pragma once

#include <routernode.h>
#include <vector>

namespace Rest
{

class Router
{
public:
    Router();
    ~Router();

    RouterNode &makeNode(const std::string &expression);

private:
    std::vector<RouterNode> nodes_;
};

}