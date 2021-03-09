#include <http/httpparser.h>

using namespace Rest;
using namespace Rest::Http;

HttpParser::HttpParser()
{

}

HttpParser::~HttpParser()
{

}

void HttpParser::clear()
{
    
}

RequestParser::ParsingState HttpParser::step(const char *data, std::size_t length)
{
    return PS_CONTINUE;
}