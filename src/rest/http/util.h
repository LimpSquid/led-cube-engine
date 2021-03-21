#pragma once

#include <http/types.h>
#include <types/type_traits.h>
#include <sstream>
#include <iostream>

namespace rest::http
{

class http_ostream : public std::ostringstream
{
public:
    void write_to(response_type &response);
};

class http_istream : public std::istringstream
{
public:
   void read_from(const request_type &request);
};

}