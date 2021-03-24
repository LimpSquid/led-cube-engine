#pragma once

#include <string>
#include <unordered_map>

namespace rest::net::routing
{

class routing_params
{
public:
    routing_params();
    ~routing_params();

    std::string get_role(const std::string &role, const std::string &def = std::string()) const;
    void set_role(const std::string &role, const std::string &value);

    void clear();

private:
    std::unordered_map<std::string, std::string> role_map_;
};

}