#pragma once

#include <util/color.h>

namespace cube::util
{

template<typename ColorValueType>
struct basic_brush
{
    using color_value_type = ColorValueType;
    using color_type = util::basic_color<color_value_type>;
};

using brush = basic_brush<unsigned char>;

}