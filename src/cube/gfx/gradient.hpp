#pragma once

#include <cube/core/color.hpp>
#include <cube/core/math.hpp>
#include <boost/container/flat_set.hpp>

namespace cube::gfx
{

struct gradient_stop
{
    gradient_stop(double p, core::color c) :
        gpos(std::clamp(p, 0.0, 1.0)),
        gcolor(std::move(c))
    { }

    double gpos;
    core::color gcolor;
};

inline bool operator<(gradient_stop const & lhs, gradient_stop const & rhs) { return core::less_than(lhs.gpos, rhs.gpos); }
inline bool operator>(gradient_stop const & lhs, gradient_stop const & rhs) { return core::greater_than(lhs.gpos, rhs.gpos); }
inline bool operator==(gradient_stop const & lhs, gradient_stop const & rhs) { return core::equal(lhs.gpos, rhs.gpos); }
inline bool operator!=(gradient_stop const & lhs, gradient_stop const & rhs) { return !core::equal(lhs.gpos, rhs.gpos); }

class gradient
{
public:
    gradient(std::initializer_list<gradient_stop> initializer_list);
    gradient();

    void add(gradient_stop stop);
    void reset();

    core::color operator()(double gpos) const;

private:
    boost::container::flat_set<gradient_stop> stops_;
};

} // End of namespace