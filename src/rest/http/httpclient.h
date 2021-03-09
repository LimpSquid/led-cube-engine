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
     * @param management The management associated to this client
     * @param socket The socket for this client
     */
    HttpClient(TcpClientManagement &management, Socket &&socket);
    
    /**
     * @brief Destroy the HttpClient object
     */
    virtual ~HttpClient() override;

private:
    virtual Rest::RequestParser &requestParser();

    HttpParser parser_;
};

}