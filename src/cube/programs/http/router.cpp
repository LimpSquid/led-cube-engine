#include <cube/programs/http/router.hpp>
#include <cube/core/logging.hpp>
#include <cube/core/utils.hpp>
#include <boost/algorithm/string.hpp>

using namespace cube::core;

namespace cube::programs::http
{

router & router::add_route(route route)
{
    if (route.location.empty())
        throw std::runtime_error("Route cannot be empty");

    if (!route.handler) {
        LOG_WRN("Request handler not set for route", LOG_ARG("location", route.location));
        route.handler = [](auto req) { return response::not_found(req); };
    }

    if (route.expected_content_type && route.expected_content_type->empty())
        throw std::runtime_error("Route expected content type cannot an empty string");

    routes_[route.location] = std::move(route);
    return *this;
}

router & router::operator()(route route)
{
    return add_route(std::move(route));
}

http_request_handler_t router::mux() const
{
    return std::bind(signature<http_request_t>::select_overload(&router::operator()),
        shared_from_this(), std::placeholders::_1);
}

http_response_t router::operator()(http_request_t req) const
{
    auto const & target = req.target();
    if(target.empty() ||
        target[0] != '/' ||
        target.find("..") != std::string_view::npos)
        return response::bad_request("Illegal request-target", req);

    auto const search = routes_.find(std::string{target});
    if (search == routes_.end())
        return response::not_found(req);

    auto const & route = search->second;
    if (!route.allowed_methods.empty() &&
        !route.allowed_methods.count(req.method()))
        return response::method_not_allowed(req);

    auto const & headers = req.base();
    auto const content_type = headers[http::field::content_type];
    if (route.expected_content_type && (content_type.empty() ||
        !boost::iequals(*route.expected_content_type, content_type)))
        return response::bad_request("Content type not allowed", req);

    return route.handler(std::move(req));
}

} // End of namespace
