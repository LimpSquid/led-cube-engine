#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <functional>

namespace cube::core { class engine_context; }
namespace cube::programs::http
{

namespace net = boost::asio;
namespace http = boost::beast::http;
using http_request_t = http::request<http::string_body>;
using http_response_t = http::response<http::string_body>;
using http_request_handler_t = std::function<http_response_t(http_request_t)>;

class listener;
class session;
class server
{
public:
    server(core::engine_context & context, net::ip::address interface, unsigned short port);
    server(server && other);

    void run(http_request_handler_t request_handler);

private:
    server(server const &) = delete;

    core::engine_context & context_;
    std::shared_ptr<listener> listener_;
    std::unordered_map<int, std::shared_ptr<session>> sessions_;

    net::ip::address interface_;
    unsigned short port_;
    unsigned int session_id_{0};
};

inline server make_server_from_string(core::engine_context & context, std::string const & address)
{
    using std::operator""s;

    auto const colon = address.find(':');
    if (colon == std::string::npos)
        throw std::runtime_error("address '"s + address + "' should contain a colon, i.e. 'interface:port'");

    auto const interface = net::ip::make_address(address.substr(0, colon).c_str());
    unsigned short port;
    try {
        port = static_cast<unsigned short>(std::stoul(address.substr(colon + 1).c_str(), nullptr, 10));
    } catch(...) {
        throw std::runtime_error("Unable to parse port");
    }

    return server{context, std::move(interface), port};
}

} // End of namespace
