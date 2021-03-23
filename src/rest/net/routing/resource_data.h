#pragma once

#include <string>
#include <unordered_map>

namespace rest::net::routing
{

class resource_data
{
public:
    resource_data();
    ~resource_data();

    std::string get_role(const std::string &role) const;
    void set_role(const std::string &role, const std::string &value);

    void clear();

private:
    std::unordered_map<std::string, std::string> role_map_;
};

}