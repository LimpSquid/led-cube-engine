#pragma once

#include <cube/core/color.hpp>
#include <boost/container/flat_set.hpp>
#include <algorithm>

namespace cube::core
{

struct gradient_stop
{
    gradient_stop(double p, color c) :
        gpos(std::clamp(p, 0.0, 1.0)),
        gcolor(std::move(c))
    { }

    double gpos;
    color gcolor;
};

inline bool operator<(gradient_stop const & lhs, gradient_stop const & rhs) { return std::less<double>{}(lhs.gpos, rhs.gpos); }
inline bool operator>(gradient_stop const & lhs, gradient_stop const & rhs) { return std::greater<double>{}(lhs.gpos, rhs.gpos); }
inline bool operator==(gradient_stop const & lhs, gradient_stop const & rhs) { return std::equal_to<double>{}(lhs.gpos, rhs.gpos); }
inline bool operator!=(gradient_stop const & lhs, gradient_stop const & rhs) { return std::not_equal_to<double>{}(lhs.gpos, rhs.gpos); }

class gradient
{
public:
    gradient();

    void add(gradient_stop stop);
    void reset();

    color operator()(double gpos);

private:
    boost::container::flat_set<gradient_stop> stops_;
};

}
