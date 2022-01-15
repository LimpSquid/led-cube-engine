#include <cube/gfx/gradient.hpp>

using namespace cube::core;

namespace cube::gfx
{

gradient::gradient(std::initializer_list<gradient_stop> initializer_list) :
    stops_(std::move(initializer_list))
{
    // Won't be inserted if already provided by initializer list
    stops_.insert({0.0, color_black});
    stops_.insert({1.0, color_black});
}

gradient::gradient() :
    gradient({})
{ }

gradient & gradient::add(gradient_stop stop)
{
    stops_.erase(stop);
    stops_.insert(std::move(stop));
    return *this;
}

boost::container::flat_set<gradient_stop> const & gradient::stops() const
{
    return stops_;
}

void gradient::reset()
{
    stops_.clear();
    add({0.0, color_black});
    add({1.0, color_black});
}

color gradient::operator()(double gpos) const
{
    gpos = std::clamp(gpos, 0.0, 1.0);
    auto gs1 = std::lower_bound(stops_.begin() + 1, stops_.end(), gpos,
        [](gradient_stop const & lhs, double rhs) { return core::less_than(lhs.gpos, rhs); });
    auto gs0 = gs1 - 1; // safe, because we always have two gradient stops and start searching from index 1

    return map(gpos, gs0->gpos, gs1->gpos, gs0->gcolor.vec(), gs1->gcolor.vec());
}

} // End of namespace
