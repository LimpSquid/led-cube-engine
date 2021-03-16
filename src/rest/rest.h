#pragma once

#include <http/types.h>
#include <http/http_server.h>
#include <http/http_handler.h>
#include <net/routing/router.h>

namespace rest
{
    // Http related types
    using http_server = http::http_server;
    using http_handler = http::http_handler;

    // Net related types
    using router = net::routing::router;
}