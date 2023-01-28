#pragma once

#include <cube/core/math.hpp>
#include <functional>

namespace cube::core
{

using parallel_range_t = range<int>; // from: inclusive, to: exclusive
using parallel_handler_t = std::function<void(parallel_range_t)>;

constexpr double use_half_cpus = 0.5;
constexpr double use_all_cpus = 0.0;

void parallel_for(parallel_range_t range, parallel_handler_t handler, double nice_factor = use_half_cpus /* [0, 1] */);

// Example 1:
//   - num CPUs: 4
//   - range: [0, 10)
//   - nice: 0.0
// -----------------------
// Thread-1 range: [0,  2)
// Thread-2 range: [2,  4)
// Thread-3 range: [4,  6)
// Thread-4 range: [6, 10)

// Example 2:
//   - num CPUs: 6
//   - range: [0, 9)
//   - nice: 0.5
// -----------------------
// Thread-1 range: [0,  3)
// Thread-2 range: [3,  6)
// Thread-3 range: [3,  9)

}
