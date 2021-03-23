#include <net/routing/resource_data.h>

using namespace rest::net::routing;

resource_data::resource_data()
{

}

resource_data::~resource_data()
{

}

void resource_data::set_role(const std::string &role, const std::string &value)
{
    role_map_[role] =  value;
}

void resource_data::clear()
{
    role_map_.clear();
}