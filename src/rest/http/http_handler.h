#pragma once

#include <http/types.h>
#include <net/routing/router_handler.h>

namespace rest::http
{

class http_handler : public net::routing::router_handler
{
public:
    http_handler();
    virtual ~http_handler() override;

    void handle(const request_type &request, response_type &response);

private:
};

};