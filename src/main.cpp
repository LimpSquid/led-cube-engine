#include <rest/rest.h>
#include <boost/make_shared.hpp>

int main(int argc, char *argv[])
{
    rest::http_server srv("127.0.0.1", "50000");
    rest::router::pointer router = rest::router::create();

    auto &handler = router->make_handler<rest::http_handler>("/test/url");
    auto &handler2 = router->make_handler<rest::http_handler>("/test/url2");

    handler.get([](const rest::http::request_type &request, rest::http::response_type &response) {
        rest::http_streamable_body body(response);

        struct
        {
            std::string serialize() const { return "world"; }
        } world;

        body << "hello" << " " << world;
    });

    handler2.get([](const rest::http::request_type &request, rest::http::response_type &response) {
        response.body() = "world";
    });

    srv.begin(router);
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
