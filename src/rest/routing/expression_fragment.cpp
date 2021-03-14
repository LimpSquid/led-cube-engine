#include <routing/expression_fragment.h>

using namespace rest::routing;

expression_fragment::~expression_fragment()
{

}

expression_fragment::expression_fragment(expression_data &data) :
    data_(data)
{

}