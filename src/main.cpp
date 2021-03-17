#include <rest/rest.h>
#include <boost/make_shared.hpp>

using namespace rest;

int main(int argc, char *argv[])
{
    rest::http_server srv("127.0.0.1", "50000");
    rest::router::pointer router = rest::router::create();

    auto &handler = router->make_handler<rest::http_handler>("/test/url");
    auto &handler2 = router->make_handler<rest::http_handler>("/test/url2");

    handler.get([](const http::request_type &request, http::response_type &response) {
        response.body() = "hello";
    });

    handler2.get([](const http::request_type &request, http::response_type &response) {
        response.body() = "world";
    });

    srv.begin(router);
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
