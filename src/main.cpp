#include <iostream>
#include <rest/router.h>
#include <rest/http/httpserver.h>
#include <boost/make_shared.hpp>

using namespace Rest;
using namespace Rest::Http;

int main(int argc, char *argv[])
{
    HttpServer srv("127.0.0.1", "50000");
    boost::shared_ptr<Router> router = boost::make_shared<Router>();

    srv.begin(router);
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
