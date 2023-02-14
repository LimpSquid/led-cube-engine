#include <cube/core/color.hpp>
#include <cube/core/math.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
#include <bit>

namespace cube::core
{

color::color(color_vec_t const & vec)
{
    auto clamp = [](auto scalar) {
        int x = static_cast<int>(std::round(scalar));
        if (x < color_min_value) return color_min_value;
        if (x > color_max_value) return color_max_value;
        return color_t(x);
    };

    r = clamp(vec.r);
    g = clamp(vec.g);
    b = clamp(vec.b);
    a = clamp(vec.a);
}

color lighter(color const & c, double factor)
{
    factor = std::clamp(factor, 0.0, 1.0);
    return map(factor, 0.0, 1.0, c.vec(), color_white.vec());
}

color darker(color const & c, double factor)
{
    factor = std::clamp(factor, 0.0, 1.0);
    return map(factor, 0.0, 1.0, c.vec(), color_black.vec());
}

color adjust_brightness(color const & c, double factor)
{
    if (less_than(factor, 0.5))
        return darker(c, map(factor, 0.0, 0.5, 0.0, 1.0));
    if (greater_than(factor, 0.5))
        return lighter(c, map(factor, 0.5, 1.0, 0.0, 1.0));
    return c;
}

color color_from_string(std::string c)
{
    if (c.empty())
        return {};

    boost::algorithm::to_lower(c);
    if (c.at(0) == '#' && c.size() == 9) { // rgba hex code, e.g. #ff0000ff (red)
        c.erase(0, 1); // Remove '#'
        rgba_t val = static_cast<rgba_t>(std::stoull(c, nullptr, 16));
        return {val};
    }

#define STATEMENT_FOR(name) \
    if (c == #name) return color_##name;
    STATEMENT_FOR(transparent)
    STATEMENT_FOR(black)
    STATEMENT_FOR(white)
    STATEMENT_FOR(red)
    STATEMENT_FOR(green)
    STATEMENT_FOR(blue)
    STATEMENT_FOR(cyan)
    STATEMENT_FOR(magenta)
    STATEMENT_FOR(yellow)
    STATEMENT_FOR(orange)
    STATEMENT_FOR(pink)
    STATEMENT_FOR(steel_blue)
#undef STATEMENT_FOR
    return {};
}

std::string color_to_string(color const & c)
{
#define STATEMENT_FOR(name) \
    if (c == color_##name) return #name;
    STATEMENT_FOR(transparent)
    STATEMENT_FOR(black)
    STATEMENT_FOR(white)
    STATEMENT_FOR(red)
    STATEMENT_FOR(green)
    STATEMENT_FOR(blue)
    STATEMENT_FOR(cyan)
    STATEMENT_FOR(magenta)
    STATEMENT_FOR(yellow)
    STATEMENT_FOR(orange)
    STATEMENT_FOR(pink)
    STATEMENT_FOR(steel_blue)
#undef STATEMENT_FOR

    std::stringstream stream;
    stream
        << "#"
        << std::setfill('0') << std::setw(sizeof(rgba_t) * 2)
        << std::hex << c.rgba();
    return stream.str();
}

void alpha_blend(color const & c, color & bucket)
{
    constexpr int shift{sizeof(color_t) * 8};

    color_t const alpha = c.a;
    color_t const inv_alpha = static_cast<color_t>(color_max_value - alpha);
    bucket.r = static_cast<color_t>((alpha * c.r + inv_alpha * bucket.r) >> shift);
    bucket.g = static_cast<color_t>((alpha * c.g + inv_alpha * bucket.g) >> shift);
    bucket.b = static_cast<color_t>((alpha * c.b + inv_alpha * bucket.b) >> shift);
    bucket.a = color_max_value;
}

void alpha_blend(rgba_t const & c, rgba_t & bucket)
{
    constexpr int shift{sizeof(color_t) * 8};

    color_t const * const c_ptr = reinterpret_cast<color_t const *>(&c);
    color_t * const bucket_ptr = reinterpret_cast<color_t *>(&bucket);

    color_t const alpha = color_t(c); // bit 0 - 7
    color_t const inv_alpha = static_cast<color_t>(color_max_value - alpha);

    if constexpr (std::endian::native == std::endian::little) {
        bucket_ptr[0] = color_max_value; // Alpha channel
        bucket_ptr[1] = static_cast<color_t>((alpha * c_ptr[1] + inv_alpha * bucket_ptr[1]) >> shift);
        bucket_ptr[2] = static_cast<color_t>((alpha * c_ptr[2] + inv_alpha * bucket_ptr[2]) >> shift);
        bucket_ptr[3] = static_cast<color_t>((alpha * c_ptr[3] + inv_alpha * bucket_ptr[3]) >> shift);
    } else {
        bucket_ptr[0] = static_cast<color_t>((alpha * c_ptr[0] + inv_alpha * bucket_ptr[0]) >> shift);
        bucket_ptr[1] = static_cast<color_t>((alpha * c_ptr[1] + inv_alpha * bucket_ptr[1]) >> shift);
        bucket_ptr[2] = static_cast<color_t>((alpha * c_ptr[2] + inv_alpha * bucket_ptr[2]) >> shift);
        bucket_ptr[3] = color_max_value; // Alpha channel
    }
}

void blend(color const & c, color & bucket)
{
    if (c.opaque())
        bucket = c;
    else
        alpha_blend(c, bucket); // blend the new color with the existing color in the bucket
}

void blend(rgba_t const & c, rgba_t & bucket)
{
    if (opaque(c))
        bucket = c;
    else
        alpha_blend(c, bucket); // blend the new color with the existing color in the bucket
}

void blend(color const & c, rgba_t & bucket)
{
    color tmp(bucket);
    blend(c, tmp);
    bucket = tmp.rgba();
}

} // End of namespace
