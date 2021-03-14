#pragma once

#include <string>
#include <vector>

namespace rest::net::routing
{

class router_node;
class router
{
public:
    router();
    ~router();

    router_node &make_node(const std::string &expression);

private:
    std::vector<router_node> nodes_;
};

}