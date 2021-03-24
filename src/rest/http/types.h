#pragma once

#include <net/routing/routing_params.h>
#include <boost/beast/http.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace rest::http
{

/**
 * @brief The http body type for requets and responses
 */
using body_type = boost::beast::http::string_body;

/**
 * @brief The http request type for reading from streamable sockets
 */
using request_type = boost::beast::http::request<body_type>;

/**
 * @brief The http responnse type for writing to streamable sockets
 */
using response_type = boost::beast::http::response<body_type>;

/**
 * @brief The http buffer type for buffering requests
 */
using buffer_type = boost::beast::flat_buffer;

/**
 * @brief The routing parameters type
 */
using routing_params_type = rest::net::routing::routing_params;

}