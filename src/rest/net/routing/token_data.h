#pragma once

#include <string>
#include <unordered_map>

namespace rest::net::routing
{

class token_data
{
public:
    token_data();
    ~token_data();

    void insert_role(const std::string &role, const std::string &value);

    void clear();

private:
    std::unordered_map<std::string, std::string> role_map_;
};

}