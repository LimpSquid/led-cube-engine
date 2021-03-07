#pragma once

#include <requestparser.h>

namespace Rest::Http
{

class HttpParser : public Rest::RequestParser
{
public:
    /**
     * @brief Construct a new HttpParser object
     */
    HttpParser();

    /**
     * @brief Destroy the HttpParser object
     */
    virtual ~HttpParser() override;

private:
    virtual void clear() override;
    virtual ParsingState step(const char *data, std::size_t length) override;
};

}