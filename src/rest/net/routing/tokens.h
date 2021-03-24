#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

namespace rest::net::routing
{
class routing_params;

class base_token
{
public:
    using pointer = boost::shared_ptr<base_token>;

    virtual bool match(const std::string &tag) const = 0;
    virtual void provide_params(const std::string &tag, routing_params &params) const;
};

class dummy_token : public base_token
{
public:
    static pointer create();
    virtual bool match(const std::string &tag) const override;
};

class matching_token : public base_token
{
public:
    static pointer create(const std::string &tag);
    virtual bool match(const std::string &tag) const override;

protected:
    matching_token(const std::string &tag);

private:
    const std::string tag_;
};

class role_token : public base_token
{
public:
    static pointer create(const std::string &role);
    virtual bool match(const std::string &tag) const override;
    virtual void provide_params(const std::string &tag, routing_params &params) const override;

protected:
    role_token(const std::string &role);

private:
    std::string role_;
};

class regex_token : public base_token
{
public:
    static pointer create(const std::string &role, const std::string &regex);
    virtual bool match(const std::string &tag) const override;
    virtual void provide_params(const std::string &tag, routing_params &params) const override;

protected:
    regex_token(const std::string &role, const std::string &regex);

private:
    std::string role_;
    std::string regex_;
};

}