#pragma once

#include <cube/core/color.hpp>
#include <chrono>
#include <variant>

namespace cube::gfx
{

using property_value = std::variant<
    int8_t, int16_t, int32_t, int64_t,
    uint8_t, uint16_t, uint32_t, uint64_t,
    float, double, long double,
    std::string,
    std::chrono::nanoseconds, std::chrono::milliseconds, std::chrono::seconds,
    core::color>;

} // End of namespace
