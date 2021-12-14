#include <cube/core/color.hpp>

namespace cube::core
{

color_scalar operator+(color_scalar const & lhs, color_scalar const & rhs)
{
    double const r = lhs.r + rhs.r;
    double const g = lhs.g + rhs.g;
    double const b = lhs.b + rhs.b;
    double const a = lhs.a + rhs.a;

    return { r, g, b, a };
}

color_scalar operator-(color_scalar const & lhs, color_scalar const & rhs)
{
    double const r = lhs.r - rhs.r;
    double const g = lhs.g - rhs.g;
    double const b = lhs.b - rhs.b;
    double const a = lhs.a - rhs.a;

    return { r, g, b, a };
}

color_scalar operator*(color_scalar const & lhs, double scalar)
{
    double const r = lhs.r * scalar;
    double const g = lhs.g * scalar;
    double const b = lhs.b * scalar;
    double const a = lhs.a * scalar;

    return { r, g, b, a };
}

color_scalar operator/(color_scalar const & lhs, double scalar)
{
    double const r = lhs.r / scalar;
    double const g = lhs.g / scalar;
    double const b = lhs.b / scalar;
    double const a = lhs.a / scalar;

    return { r, g, b, a };
}

void alpha_blend(color const & c, color & bucket)
{
    constexpr int shift = sizeof(color_t) * 8;

    color_t const alpha = c.a;
    color_t const inv_alpha = color_max_value - alpha;
    bucket.r = (alpha * c.r + inv_alpha * bucket.r) >> shift;
    bucket.g = (alpha * c.g + inv_alpha * bucket.g) >> shift;
    bucket.b = (alpha * c.b + inv_alpha * bucket.b) >> shift;
    bucket.a = color_max_value;
}

void blend(color const & c, color & bucket)
{
    // Two special cases are handled here:
    // - In case 'c' is color_clear, we will "clear" the bucket
    // - In case 'bucket' is cleared, then we will fill it with color 'c' without blending
    if (c == color_clear || bucket == color_clear)
        bucket = c;
    else
        alpha_blend(c, bucket); // blend the new color with the existing color in the bucket
}

void blend(color const & c, rgba_t & bucket)
{
    color tmp{bucket};
    blend(c, tmp);
    bucket = tmp.rgba();
}

} // end of namespace
