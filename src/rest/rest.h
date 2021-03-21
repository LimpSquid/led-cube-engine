#pragma once

#include <http/types.h>
#include <http/http_server.h>
#include <http/http_handler.h>
#include <http/util.h>
#include <net/routing/router.h>

namespace rest
{
    // Http related types
    using http_server = http::http_server;
    using http_handler = http::http_handler;
    using http_ostream = http::http_ostream;
    using http_istream = http::http_istream;

    // Routing related types
    using router = net::routing::router;
}