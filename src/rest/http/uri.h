#pragma once

#include <string>

namespace rest::http
{

class uri
{
public:
    uri() = default;
    uri(const std::string &raw);
    uri(const uri &other) = default;
    uri(uri &&other) = default;

    uri &operator=(const uri &other) = default;
    uri &operator=(uri &&other) = default;

private:
    void parse_raw();

    std::string raw_;
    std::string scheme_;
    std::string authority_;
    std::string path_;
    std::string query_;
    std::string fragment_;
};

}