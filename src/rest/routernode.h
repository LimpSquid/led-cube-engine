#pragma once

#include <string>
#include <vector>

namespace Rest
{

class RouterNode
{
public:
    RouterNode(const std::string &expression);
    RouterNode(RouterNode &&other);
    ~RouterNode();

    bool match(const std::string &endpoint) const;

    bool operator==(const RouterNode &other) const;
    bool operator!=(const RouterNode &other) const;

private:
    const std::size_t hash_;
};

}