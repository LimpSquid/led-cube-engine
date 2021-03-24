#pragma once

#include <string>

namespace rest::net
{

class uri
{
public:
    uri() = default;
    uri(const std::string &raw);
    uri(const uri &other) = default;
    uri(uri &&other) = default;

    bool valid() const;
    const std::string &raw() const;
    const std::string &scheme() const;
    const std::string &authority() const;
    const std::string &path() const;
    const std::string &query() const;
    const std::string &fragment() const;

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
    bool valid_;
};

}