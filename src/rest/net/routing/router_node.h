#pragma once

#include <net/routing/tokens.h>
#include <string>
#include <vector>

namespace rest::net::routing
{

class token_data;
class router_node
{
public:
    router_node(const std::string &url_expression);
    router_node(router_node &&other);
    ~router_node();

    bool match(const std::string &url, token_data &data) const;

    bool operator==(const router_node &other) const;
    bool operator!=(const router_node &other) const;

private:
    void tokenize(const std::string &url_expression);

    const std::size_t url_expr_hash_;
    
    std::vector<base_token::pointer> tokens_;
};

}