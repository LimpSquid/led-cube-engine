#include <net/routing/tokens.h>
#include <net/routing/resource_data.h>

using namespace rest::net::routing;

void base_token::provide_data(const std::string &tag, resource_data &) const
{

}

base_token::pointer dummy_token::create()
{
    return pointer(new dummy_token);
}

bool dummy_token::match(const std::string &) const
{
    return true;
}

base_token::pointer matching_token::create(const std::string &tag)
{
    return pointer(new matching_token(tag));
}

bool matching_token::match(const std::string &tag) const
{
    return tag_ == tag;
}

matching_token::matching_token(const std::string &tag) :
    tag_(tag)
{

}

base_token::pointer role_token::create(const std::string &role)
{
    return pointer(new role_token(role));
}

bool role_token::match(const std::string &tag) const
{
    return !tag.empty();
}

void role_token::provide_data(const std::string &tag, resource_data &data) const
{
    data.set_role(role_, tag);
}

role_token::role_token(const std::string &role) :
    role_(role)
{

}

base_token::pointer regex_token::create(const std::string &role, const std::string &regex)
{
    return pointer(new regex_token(role, regex));
}

bool regex_token::match(const std::string &tag) const
{
    return false;
}

void regex_token::provide_data(const std::string &tag, resource_data &data) const
{
    data.set_role(role_, tag);
}

regex_token::regex_token(const std::string &role, const std::string &regex) :
    role_(role),
    regex_(regex)
{

}
