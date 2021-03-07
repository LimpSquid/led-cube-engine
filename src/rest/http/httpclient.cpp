#include <http/httpclient.h>

using namespace Rest::Http;

HttpClient::HttpClient(const Context &context) :
    TcpClient(context)
{

}

HttpClient::~HttpClient()
{
    
}

Rest::RequestParser &HttpClient::requestParser()
{
    return parser_;
}