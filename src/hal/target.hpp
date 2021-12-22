#pragma once

#ifdef TARGET_MOCK
#include "mock/fwd.hpp"
#elif TARGET_RPI
#include "rpi/fwd.hpp"
#else
#error "Oopsie, it seems like you forgot to expose your target here."
#endif
