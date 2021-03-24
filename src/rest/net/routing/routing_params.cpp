#include <net/routing/routing_params.h>

using namespace rest::net::routing;

routing_params::routing_params()
{

}

routing_params::~routing_params()
{

}

std::string routing_params::get_role(const std::string &role, const std::string &def) const
{
    auto search = role_map_.find(role);

    return search == role_map_.cend() ? def : search->second;
}

void routing_params::set_role(const std::string &role, const std::string &value)
{
    role_map_[role] =  value;
}

void routing_params::clear()
{
    role_map_.clear();
}