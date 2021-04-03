#include <rest/rest.h>
#include <cube/core/engine.h>
#include <cube/hal/voxel_display.h>
#include <sstream>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/make_shared.hpp>

// This is all temporary stuffs, just playing in my sandbox

struct world
{
    friend std::ostream& operator<<(std::ostream &out, const world &)
    {
        return out << "world";
    }
};

void run_server()
{
    rest::http_server srv("127.0.0.1", "50000");
    srv.install_plugin([](const rest::http::request_type &request, const rest::http::response_type &response) {
        std::stringstream stream;

	    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%d/%b/%Y:%H:%M:%S");
	    stream.imbue(std::locale(stream.getloc(), facet));

        stream << boost::posix_time::second_clock::local_time() << " request: \"" <<
            request.method() << " " << request.base().target() << "\" version: \"" <<
            request.base().version() << "\" response: \"" << response.body() << "\"";

        std::cout << stream.str() << std::endl;
    });

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
}

int main(int argc, char *argv[])
{
    cube::core::engine cube_engine(new cube::hal::voxel_display);

    run_server();
    return EXIT_SUCCESS;
}
