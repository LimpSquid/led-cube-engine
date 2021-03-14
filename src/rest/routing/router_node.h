#pragma once

#include <routing/tokens.h>
#include <string>
#include <vector>

namespace rest::routing
{

class token_data;
class router_node
{
public:
    router_node(const std::string &expression);
    router_node(router_node &&other);
    ~router_node();

    bool match(const std::string &endpoint, token_data &data) const;

    bool operator==(const router_node &other) const;
    bool operator!=(const router_node &other) const;

private:
    void tokenize(const std::string &expression);

    const std::size_t expr_hash_;
    
    std::vector<base_token::pointer> tokens_;
};

}