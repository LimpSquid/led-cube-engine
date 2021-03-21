#include <http/util.h>
#include <utility>

using namespace rest::http;

void http_ostream::write_to(response_type &response)
{
    response.body() = std::move(str());
}

void http_istream::read_from(const request_type &request)
{
    str(request.body());
}