#pragma once

#include <tcpclient.h>
#include <boost/beast/http.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace Rest::Http
{

class HttpClient : public Rest::TcpClient
{
public:
    /**
     * @brief The request body type for a HttpClient object
     */
    using HttpRequestBody = boost::beast::http::string_body;

    /**
     * @brief The request type for a HttpClient object
     */
    using HttpRequest = boost::beast::http::request<HttpRequestBody>;

    /**
     * @brief The buffer type for a HttpClient object
     */
    using HttpBuffer = boost::beast::flat_buffer;

    /**
     * @brief Construct a new HttpClient object
     * @param management The management associated to this client
     * @param socket The socket for this client
     */
    HttpClient(TcpClientManagement &management, Socket &&socket);
    
    /**
     * @brief Destroy the HttpClient object
     */
    virtual ~HttpClient() override;

private:
    virtual void stateChanged(const ClientState &state) override;
    void asyncRead();
    void httpRead(boost::beast::error_code error);

    HttpRequest request_;
    HttpBuffer buffer_;
};

}