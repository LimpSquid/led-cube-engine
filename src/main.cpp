#include <rest/rest.h>
#include <boost/make_shared.hpp>

struct world
{
    friend std::ostream& operator<<(std::ostream &out, const world &)
    {
        return out << "world";
    }
};

int main(int argc, char *argv[])
{
    rest::http_server srv("127.0.0.1", "50000");
    rest::router::pointer router = rest::router::create();

    auto &handler = router->make_handler<rest::http_handler>(R"(/api/[version=^[0-9]{1,}$]/resource)");
    auto &kirby = router->make_handler<rest::http_handler>("/api/<other>/resource");

    handler.install_get([](const rest::http::routing_params_type &params, const rest::http::request_type &request, rest::http::response_type &response) {
        rest::http_ostream stream;

        stream << "hello " << world() << ", verb: get, api version: " << params.get_role("version");
        stream.write_to(response);
    }).install_post([](const rest::http::routing_params_type &params, const rest::http::request_type &request, rest::http::response_type &response) {
        rest::http_ostream stream;

        stream << "hello " << world() << ", verb: post, api version: " << params.get_role("version");
        stream.write_to(response);
    });

    kirby.install_get([](const rest::http::routing_params_type &params, const rest::http::request_type &request, rest::http::response_type &response) {
        rest::http_ostream stream;

        stream << "Kirby ate: " << params.get_role("other");
        stream.write_to(response);
    });


    srv.begin(router);
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
