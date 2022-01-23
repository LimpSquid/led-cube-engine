#pragma once

#include <cube/core/math.hpp>
#include <functional>

namespace cube::core
{

using parallel_exclusive_range_t = range<int>;
using parallel_handler_t = std::function<void(parallel_exclusive_range_t)>;

void parallel_for(parallel_exclusive_range_t range, parallel_handler_t handler, double nice_factor = 0.0);

}
