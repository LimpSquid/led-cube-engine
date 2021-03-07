#pragma once

#include <tcpclient.h>
#include <http/httpparser.h>

namespace Rest::Http
{

class HttpClient : public Rest::TcpClient
{
public:
    /**
     * @brief Construct a new HttpClient object
     * @param context The context for the TcpClient
     */
    HttpClient(const Context &context);

    /**
     * @brief Destroy the HttpClient object
     */
    virtual ~HttpClient() override;

private:
    virtual Rest::RequestParser &requestParser();

    HttpParser parser_;
};

}