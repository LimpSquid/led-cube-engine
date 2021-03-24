#pragma once

#include <http/types.h>
#include <net/routing/router_handler.h>
#include <unordered_map>
#include <functional>

namespace rest::http
{

class http_handler : public net::routing::router_handler
{
public:
    using handle_callback = std::function<void(const routing_params_type &, const request_type &, response_type &)>;

    http_handler();
    virtual ~http_handler() override;

    void handle(const routing_params_type &params, const request_type &request, response_type &response);

    http_handler &get(const handle_callback &callback);

private:
    std::unordered_map<boost::beast::http::verb, handle_callback> verb_mapping;
};

};