#include <net/routing/router_node.h>
#include <boost/tokenizer.hpp>
#include <functional>
#include <stdexcept>

using namespace rest::net::routing;

router_node::router_node(const std::string &resource_expression, const router_handler::pointer &handler) :
    resource_expr_hash_(std::hash<std::string>{ }(resource_expression)),
    handler_(handler)
{
    tokenize(resource_expression);
}

router_node::~router_node()
{

}

router_handler &router_node::handler()
{
    return *handler_;
}

bool router_node::match(const std::string &resource, resource_data &data) const
{
    boost::tokenizer tags(resource, boost::escaped_list_separator<char>('\\', '/'));
    const std::size_t size = std::distance(tags.begin(), tags.end());

    // No need to match when size does not match
    if(tokens_.size() != size)
        return false;

    // Match each tag with token
    auto tag_it = tags.begin();
    auto token_it = tokens_.cbegin();
    for(; token_it != tokens_.cend(); tag_it++, token_it++) {
        const auto &tag = *tag_it;
        const auto &token = *token_it;

        if(!token->match(tag))
            return false;
    }

    // If all tokens match, provide data
    for(tag_it = tags.begin(), token_it = tokens_.cbegin(); token_it != tokens_.cend(); tag_it++, token_it++) {
        const auto &tag = *tag_it;
        const auto &token =  *token_it;

        token->provide_data(tag, data);
    }

    return true;
}

bool router_node::operator==(const router_node &other) const
{
    return (resource_expr_hash_ == other.resource_expr_hash_);
}

bool router_node::operator!=(const router_node &other) const
{
    return !operator==(other);
}

void router_node::tokenize(const std::string &resource_expression)
{
    boost::tokenizer tags(resource_expression, boost::escaped_list_separator<char>('\\', '/'));
    base_token::pointer token;

    for(const std::string &tag : tags) {
        if(tag.empty())
            token = dummy_token::create();
        if(tag.size() <= 2)
            token = matching_token::create(tag);
        else {
            const char first = tag.at(0);
            const char last = tag.at(tag.length() - 1);

            if('<' == first && '>' == last)
                token = role_token::create(tag.substr(1, tag.length() - 2));
            else if('[' == first && ']' == last) {
                std::string sub = tag.substr(1, tag.length() - 2);
                const std::size_t index = sub.find('=');

                token = regex_token::create(sub.substr(0, index),
                                            sub.substr(index + 1, sub.length() - index - 1));
            } else
                token = matching_token::create(tag);
        }

        tokens_.push_back(token);
    }
}