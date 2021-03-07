#include <requestparser.h>

using namespace Rest;

RequestParser::~RequestParser()
{
    state_ = PS_UNKNOWN;
}

RequestParser::RequestParser()
{

}

void RequestParser::reset()
{
    state_ = PS_UNKNOWN;

    clear();
}

RequestParser::ParsingState RequestParser::parse(const char *data, std::size_t length)
{
    state_ = step(data, length);
    return state_;
}

void RequestParser::clear()
{
    // Do nothing
}

RequestParser::ParsingState RequestParser::state() const
{
    return state_;
}
