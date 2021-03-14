#include <net/routing/token_data.h>

using namespace rest::net::routing;

token_data::token_data()
{

}

token_data::~token_data()
{

}

void token_data::insert_role(const std::string &role, const std::string &value)
{
    role_map_[role] =  value;
}

void token_data::clear()
{
    role_map_.clear();
}