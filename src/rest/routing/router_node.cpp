#include <routing/router_node.h>
#include <functional>
#include <boost/tokenizer.hpp>
#include <stdexcept>

using namespace rest::routing;

router_node::router_node(const std::string &expression) :
    expr_hash_(std::hash<std::string>{ }(expression))
{
    tokenize(expression);
}

router_node::router_node(router_node &&other) :
    expr_hash_(other.expr_hash_)
{

}

router_node::~router_node()
{

}

bool router_node::match(const std::string &endpoint, token_data &data) const
{
    boost::tokenizer tags(endpoint, boost::char_separator<char>("/"));
    const std::size_t size = std::distance(tags.begin(), tags.end());

    // No need to match when size does not match
    if(tokens_.size() != size)
        return false;    

    // Match each tag with token
    auto tag_it = tags.begin();
    auto token_it = tokens_.cbegin();
    for(; token_it != tokens_.cend(); tag_it++, token_it++) {
        const auto &tag = *tag_it;
        const auto &token =  *token_it;
        
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
    return (expr_hash_ == other.expr_hash_);
}

bool router_node::operator!=(const router_node &other) const
{
    return !operator==(other);
}

void router_node::tokenize(const std::string &expression)
{
    boost::tokenizer tags(expression, boost::escaped_list_separator<char>('\\', '/'));
    base_token::pointer token;

    for(const std::string &tag : tags) {
        if(tag.empty())
            throw std::invalid_argument("Expression is not allowed to have empty tags");
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