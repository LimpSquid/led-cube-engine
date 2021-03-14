#include <rest/http/http_server.h>
#include <rest/routing/router.h>
#include <boost/make_shared.hpp>

int main(int argc, char *argv[])
{
    rest::http::http_server srv("127.0.0.1", "50000");
    boost::shared_ptr<rest::routing::router> router = boost::make_shared<rest::routing::router>();

    srv.begin(router);
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
