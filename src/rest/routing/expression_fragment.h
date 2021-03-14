#pragma once

namespace rest::routing
{

class expression_data;
class expression_fragment
{
public:
    virtual ~expression_fragment();

protected:
    expression_fragment(expression_data &data);

private:
    expression_data &data_;
};

}