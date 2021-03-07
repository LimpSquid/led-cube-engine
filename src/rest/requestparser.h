#pragma once

#include <cstddef>

namespace Rest
{

class RequestParser
{
public:
    /**
     * @brief The ParsingState enum
     */
    enum ParsingState
    {
        PS_UNKNOWN = 0,     /**< Describes that the parsing state is not known */
        PS_CONTINUE,        /**< Describes that parsing is not done yet and expects more data */
        PS_FINISHED,        /**< Describes that parsing is finished */
        PS_ERROR,           /**< Describes that parsing failed */
    };

    /**
     * @brief Destroy the RequestParser object
     */
    virtual ~RequestParser();

    /**
     * @brief Get the current state of the parser
     * @return Returns ParsingState
     */
    ParsingState state() const;
    
    /**
     * @brief Reset the parser
     * Resets the parser, this method is usually called before parsing new data
     */
    void reset();

    /**
     * @brief Parse raw data
     * Parse the raw data, or a chunk of raw data
     * @param data The data to parse
     * @param length The length of the data to parse
     * @return Returns ParsingState
     */
    ParsingState parse(const char *data, std::size_t length);

protected:
    /**
     * @brief Construct a new RequestParser object
     */
    RequestParser();

    /**
     * @brief Clear the internal state of the parser
     * A method that may be overridden to clear additional data when the reset() method is called
     */
    virtual void clear();

    /**
     * @brief Execute a parse step
     * A method that must be overridden to parse (partial data) and return a new state of the parser
     * @param data The data to parse
     * @param length The length of the data to parse
     * @return Returns ParsingState
     */
    virtual ParsingState step(const char *data, std::size_t length) = 0;

private:
    ParsingState state_;
};

}