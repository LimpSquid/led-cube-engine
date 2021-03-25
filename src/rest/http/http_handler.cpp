#include <http/http_handler.h>

#define MAKE_INSTALL_NAMED(http_verb)   MAKE_INSTALL(http_verb, http_verb)
#define MAKE_INSTALL(http_verb, name)                                               \
    http_handler &http_handler::install_##name(const handle_callback &callback)     \
    {                                                                               \
        verb_mapping[beast::http::verb::http_verb] = callback;                      \
        return *this;                                                               \
    }

using namespace rest::http;
using namespace boost;

http_handler::http_handler()
{

}

http_handler::~http_handler()
{

}

void http_handler::handle(const routing_params_type &params, const request_type &request, response_type &response)
{
    const auto not_found = [&request, &response](std::string &&why)
    {
        response = { beast::http::status::not_found, request.version() };
        response.set(beast::http::field::content_type, "text/plain");
        response.body() = std::move(why);
    };

    const auto ok_callback = [&params, &request, &response](const handle_callback &callback)
    {
        response = { beast::http::status::ok, request.version() };
        response.set(beast::http::field::content_type, "text/plain");
        callback(params, request, response);
    };

    const auto search = verb_mapping.find(request.base().method());
    if(search == verb_mapping.end())
        return not_found("Requested verb for resource does not exist");

    ok_callback(search->second);
}

// Generate install methods
MAKE_INSTALL_NAMED(get)
MAKE_INSTALL_NAMED(post)
MAKE_INSTALL_NAMED(put)
MAKE_INSTALL(delete_, delete)