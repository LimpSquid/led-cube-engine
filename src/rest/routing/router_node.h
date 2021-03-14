#pragma once

#include <routing/expression_data.h>
#include <string>
#include <vector>

namespace rest::routing
{

class expression_fragment;
class router_node
{
public:
    router_node(const std::string &expression);
    router_node(router_node &&other);
    ~router_node();

    bool match(const std::string &endpoint) const;

    bool operator==(const router_node &other) const;
    bool operator!=(const router_node &other) const;

private:
    const std::size_t expr_hash_;
    
    std::vector<expression_fragment> expr_fragments_;
    expression_data expr_data_;
};

}