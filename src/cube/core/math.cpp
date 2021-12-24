#include <cube/core/math.hpp>
#include <ctime>
#include <cstdlib>

namespace
{

struct rand_seeder
{
    rand_seeder() { std::srand(std::time(nullptr)); }
};

[[gnu::used]] rand_seeder const seeder_;

} // End of namespace
