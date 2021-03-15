#pragma once

#include <net/routing/router_handler.h>

namespace rest::http
{

class http_handler : public net::routing::router_handler
{
public:
    http_handler();
    virtual ~http_handler() override;

    void handle();

private:
};

};