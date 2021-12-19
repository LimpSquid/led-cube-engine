#include <cube/core/color.hpp>
#include <cube/core/math.hpp>

namespace cube::core
{

color::color(color_vec_t const & vec)
{
    auto clamp = [](auto scalar) {
        int x = std::round(scalar);
        if (x < color_min_value) return color_min_value;
        if (x > color_max_value) return color_max_value;
        return color_t(x);
    };

    r = clamp(vec.r);
    g = clamp(vec.g);
    b = clamp(vec.b);
    a = clamp(vec.a);
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

void alpha_blend(rgba_t const & c, rgba_t & bucket)
{
    constexpr int shift = sizeof(color_t) * 8;
    color_t const * const c_ptr = reinterpret_cast<color_t const *>(&c);
    color_t * const bucket_ptr = reinterpret_cast<color_t *>(&bucket);

    color_t const alpha = c_ptr[3];
    color_t const inv_alpha = color_max_value - alpha;
    bucket_ptr[0] = (alpha * c_ptr[0] + inv_alpha * bucket_ptr[0]) >> shift;
    bucket_ptr[1] = (alpha * c_ptr[1] + inv_alpha * bucket_ptr[1]) >> shift;
    bucket_ptr[2] = (alpha * c_ptr[2] + inv_alpha * bucket_ptr[2]) >> shift;
    bucket_ptr[3] = color_max_value;
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

void blend(rgba_t const & c, rgba_t & bucket)
{
    // Same as above
    if (c == color_clear.rgba() || bucket == color_clear.rgba())
        bucket = c;
    else
        alpha_blend(c, bucket);
}

void blend(color const & c, rgba_t & bucket)
{
    color tmp(bucket);
    blend(c, tmp);
    bucket = tmp.rgba();
}

} // End of namespace
