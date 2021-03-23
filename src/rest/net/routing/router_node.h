#pragma once

#include <net/routing/tokens.h>
#include <net/routing/router_handler.h>
#include <string>
#include <vector>

namespace rest::net::routing
{

class resource_data;
class router_node
{
public:
    /**
     * @brief Construct a new router_node object
     * Constructs a new router_node object from a given resource expression. The resource expression
     * consists of tags separated by a '/' character, for example \b /rest/api/v1/endpoint.
     * Each tag (highlighted in bold) is tokenized per the following format:
     * - /rest/api/<strong>tag</strong>/endpoint: matching_token
     * - /rest/api/<strong><role></strong>/endpoint: role_token
     * - /rest/api/<strong>[role=regex]</strong>/endpoint: regex_token
     *
     * @param resource_expression The expression specified by a resource path, e.g. \b /rest/api/v1/endpoint \b
     * @param handler The router handler
     */
    router_node(const std::string &resource_expression, const router_handler::pointer &handler);
    router_node(const router_node &other) = default;
    router_node(router_node &&other) = default;
    ~router_node();

    router_handler &handler();
    bool match(const std::string &resource, resource_data &data) const;

    router_node &operator=(const router_node &other) = default;
    router_node &operator=(router_node &&other) = default;
    bool operator==(const router_node &other) const;
    bool operator!=(const router_node &other) const;

private:
    void tokenize(const std::string &resource_expression);

    const std::size_t resource_expr_hash_;

    std::vector<base_token::pointer> tokens_;
    router_handler::pointer handler_;
};

}