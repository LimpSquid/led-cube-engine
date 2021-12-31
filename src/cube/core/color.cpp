#include <cube/core/color.hpp>
#include <cube/core/math.hpp>
#include <boost/algorithm/string.hpp>

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

color from_string(std::string c)
{
    boost::algorithm::to_lower(c);

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
#undef STATEMENT_FOR
    return {};
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

    color_t const alpha = c_ptr[3];
    color_t const inv_alpha = static_cast<color_t>(color_max_value - alpha);
    bucket_ptr[0] = static_cast<color_t>((alpha * c_ptr[0] + inv_alpha * bucket_ptr[0]) >> shift);
    bucket_ptr[1] = static_cast<color_t>((alpha * c_ptr[1] + inv_alpha * bucket_ptr[1]) >> shift);
    bucket_ptr[2] = static_cast<color_t>((alpha * c_ptr[2] + inv_alpha * bucket_ptr[2]) >> shift);
    bucket_ptr[3] = color_max_value;
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
