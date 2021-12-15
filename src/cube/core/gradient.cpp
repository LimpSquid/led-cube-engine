#include <cube/core/gradient.hpp>

namespace cube::core
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
{

}

void gradient::add(gradient_stop stop)
{
    stops_.erase(stop);
    stops_.insert(std::move(stop));
}

void gradient::reset()
{
    stops_.clear();
    add({0.0, color_black});
    add({1.0, color_black});
}

color gradient::operator()(double gpos)
{
    gpos = std::clamp(gpos, 0.0, 1.0);
    auto gs1 = std::lower_bound(stops_.begin() + 1, stops_.end(), gpos,
        [](gradient_stop const & lhs, double rhs) { return std::less<double>{}(lhs.gpos, rhs); });
    auto gs0 = gs1 - 1; // safe, because we always have two gradient stops and start searching from index 1

    return gs0->gcolor + ((gs1->gcolor - gs0->gcolor) * (gpos - gs0->gpos)) / (gs1->gpos - gs0->gpos);
}

} // End of namespace
