#include <rest/http/http_server.h>
#include <rest/net/routing/router.h>
#include <boost/make_shared.hpp>

using namespace rest;
using namespace rest::net;

int main(int argc, char *argv[])
{
    http::http_server srv("127.0.0.1", "50000");
    boost::shared_ptr<routing::router> router = boost::make_shared<routing::router>();

    router->make_node("/test/<id>");

    srv.begin(router);
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
