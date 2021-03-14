#pragma once

#include <net/tcp_client.h>
#include <boost/beast/http.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace rest::net::routing
{
    class router;
}

namespace rest::http
{

class http_client : public rest::net::tcp_client
{
public:
    /**
     * @brief The request body type for a http_client object
     */
    using http_request_body = boost::beast::http::string_body;

    /**
     * @brief The request type for a http_client object
     */
    using http_request = boost::beast::http::request<http_request_body>;

    /**
     * @brief The buffer type for a http_client object
     */
    using http_buffer = boost::beast::flat_buffer;

    /**
     * @brief Construct a new http_client object
     * @param management The management associated to this client
     * @param socket The socket for this client
     */
    http_client(rest::net::tcp_client_management &management, socket_type &&socket);
    
    /**
     * @brief Destroy the http_client object
     */
    virtual ~http_client() override;

private:
    virtual void state_changed(const client_state &state) override;
    void async_read();
    void http_read(boost::beast::error_code error);

    boost::shared_ptr<rest::net::routing::router> router_;
    http_request request_;
    http_buffer buffer_;
};

}