#pragma once

#include <http/http_handler.h>
#include <boost/beast/http.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace rest::http
{

/**
 * @brief The http handler type for the router
 */
using handler_type = http_handler;

/**
 * @brief The http body type for requets and responses
 */
using body_type = boost::beast::http::string_body;

/**
 * @brief The http request type for streaming from sockets
 */
using request_type = boost::beast::http::request<body_type>;

/**
 * @brief The http buffer type for buffering requests
 */
using buffer_type = boost::beast::flat_buffer;

}