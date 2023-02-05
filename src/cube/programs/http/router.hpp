#pragma once

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <memory>
#include <optional>
#include <unordered_set>
#include <unordered_map>

namespace cube::programs::http
{

namespace http = boost::beast::http;
using http_request_t = http::request<http::string_body>;
using http_response_t = http::response<http::string_body>;
using http_request_handler_t = std::function<http_response_t(http_request_t)>;
using http::verb;

namespace mime_type
{

inline char const * const text_html    = "text/html";
inline char const * const text_plain   = "text/plain";
inline char const * const text_css     = "text/css";

inline char const * const image_png    = "image/png";
inline char const * const image_jpeg   = "image/jpeg";
inline char const * const image_gif    = "image/gif";
inline char const * const image_svg    = "image/svg+xml";

inline char const * const application_javascript   = "application/javascript";
inline char const * const application_json         = "application/json";
inline char const * const application_xml          = "application/xml";

} // End of namespace

namespace response
{

inline http_response_t ok(http_request_t const & req)
{
    http_response_t res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type::text_plain);
    res.keep_alive(req.keep_alive());
    res.body() = "";
    res.prepare_payload();
    return res;
}

inline http_response_t bad_request(std::string why, http_request_t const & req)
{
    http_response_t res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type::text_plain);
    res.keep_alive(req.keep_alive());
    res.body() = std::move(why);
    res.prepare_payload();
    return res;
}

inline http_response_t not_found(http_request_t const & req)
{
    http_response_t res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type::text_plain);
    res.keep_alive(req.keep_alive());
    res.body() = "";
    res.prepare_payload();
    return res;
}

inline http_response_t method_not_allowed(http_request_t const & req)
{
    http_response_t res{http::status::method_not_allowed, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type::text_plain);
    res.keep_alive(req.keep_alive());
    res.body() = "";
    res.prepare_payload();
    return res;
}

} // End of namespace

struct route
{
    // required
    std::string location;
    http_request_handler_t handler;

    // optional
    std::unordered_set<http::verb> allowed_methods = {}; // empty means any method
    std::optional<std::string> expected_content_type = {}; // empty means any content type
};

class router :
    public std::enable_shared_from_this<router>
{
public:
    template<typename ... A>
    static std::shared_ptr<router> create(A && ... args)
    {
        return std::shared_ptr<router>(new router(std::forward<A>(args) ...));
    }

    router & add_route(route route);
    router & operator()(route route);

    http_request_handler_t mux() const;

private:
    router() = default;

    http_response_t operator()(http_request_t req) const;

    std::unordered_map<std::string, route> routes_;
};

} // End of namespace
