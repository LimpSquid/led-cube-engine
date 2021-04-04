#include <net/routing/tokens.h>
#include <net/routing/routing_params.h>
#include <regex>

using namespace rest::net::routing;
using namespace boost;

void base_token::provide_params(const std::string &, routing_params &) const
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

void role_token::provide_params(const std::string &tag, routing_params &params) const
{
    params.set_role(role_, tag);
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
    const std::regex tag_regex(regex_, std::regex::extended);

    return std::regex_match(tag, tag_regex);
}

void regex_token::provide_params(const std::string &tag, routing_params &params) const
{
    params.set_role(role_, tag);
}

regex_token::regex_token(const std::string &role, const std::string &regex) :
    role_(role),
    regex_(regex)
{

}
