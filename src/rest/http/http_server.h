#pragma once

#include <http/types.h>
#include <net/tcp_server.h>
#include <vector>

namespace rest::http
{

class http_server : public rest::net::tcp_server
{
public:
    using request_plugin = std::function<void(const request_type &)>;
    using response_plugin = std::function<void(const request_type &, const response_type &)>;

    /**
     * @brief Construct a new http_server object
     * @param address The address to listen on for incoming connections
     * @param port The port to listen on for incoming connections
     */
    http_server(const std::string &address, const std::string &port);

    /**
     * @brief Construct a new http_server object
     * @param endpoint The endpoint to listen on for incoming connections
     */
    http_server(const boost::asio::ip::tcp::endpoint &endpoint);

    /**
     * @brief Destroy the http_server object
     */
    virtual ~http_server() override;

    /**
     * @brief Install a plugin
     * Installs a request_plugin on the server
     * @param plugin The plugin to install
     */
    void install_plugin(const request_plugin &plugin);

        /**
     * @brief Install a plugin
     * Installs a response_plugin on the server
     * @param plugin The plugin to install
     */
    void install_plugin(const response_plugin &plugin);

private:
    virtual rest::net::tcp_client::pointer create_client(rest::net::tcp_client_management &management, socket_type &&socket) override;

    void on_signal_request(const request_type &request);
    void on_signal_response(const request_type &request, const response_type &response);

    std::vector<request_plugin> request_plugins_;
    std::vector<response_plugin> response_plugins_;
};

}