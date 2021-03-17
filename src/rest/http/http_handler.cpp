#include <http/http_handler.h>

using namespace rest::http;
using namespace boost;

http_handler::http_handler()
{

}

http_handler::~http_handler()
{

}

void http_handler::handle(const request_type &request, response_type &response)
{
    const auto not_found = [&request, &response](std::string &&why)
    {
        response = { beast::http::status::not_found, request.version() };
        response.set(beast::http::field::content_type, "text/plain");
        response.body() = std::move(why);
    };

    const auto ok_callback = [&request, &response](const handle_callback &callback)
    {
        response = { beast::http::status::ok, request.version() };
        response.set(beast::http::field::content_type, "text/plain");
        callback(request, response);
    };

    const auto search = verb_mapping.find(request.base().method());
    if(search == verb_mapping.end())
        return not_found("Requested verb for url does not exist");

    ok_callback(search->second);
}

http_handler &http_handler::get(const handle_callback &callback)
{
    verb_mapping[beast::http::verb::get] = callback;
    return *this;
}