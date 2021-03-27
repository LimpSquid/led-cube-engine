#include <util/time_step.h>

using namespace cube::util;

time_step::time_step(const time_unit &unit) :
    unit_(unit)
{

}

void time_step::update(double time)
{
    elapsed_ = time - previous_;
    previous_ = time;
}