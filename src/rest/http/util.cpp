#include <http/util.h>
#include <utility>

using namespace rest::http;

streamable_body::streamable_body(response_type &response) :
    body_(response.body())
{

}

streamable_body::~streamable_body()
{

}

streamable_body &streamable_body::operator<<(const std::string &string)
{
    body_.append(string);
    return *this;
}

streamable_body &streamable_body::operator<<(std::string &&string)
{
    body_.append(std::move(string));
    return *this;
}