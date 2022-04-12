#pragma once

#include <cube/gfx/gradient.hpp>
#include <cube/core/json_utils.hpp>

namespace cube::gfx
{

using cube::core::to_json;
using cube::core::from_json;
using cube::core::parse_field;

inline void to_json(gradient_stop const & stop, nlohmann::json & out)
{
    out["stop_position"] = stop.gpos;
    to_json(stop.gcolor, out["stop_color"]);
}

inline void from_json(nlohmann::json const & json, gradient_stop & out)
{
    using std::operator""s;

    if (!json.is_object())
        throw std::invalid_argument("Expected JSON object got: "s + json.type_name());

    out.gpos = parse_field<double>(json, "stop_position");
    from_json(json.at("stop_color"), out.gcolor);
}

inline void to_json(gradient const & g, nlohmann::json & out)
{
    to_json(g.stops(), out["gradient_stops"]);
}

inline void from_json(nlohmann::json const & json, gradient & out)
{
    using std::operator""s;

    if (!json.is_object())
        throw std::invalid_argument("Expected JSON object got: "s + json.type_name());

    std::vector<gradient_stop> stops;
    from_json(json.at("gradient_stops"), stops);

    for (auto const & stop : stops)
        out.add(stop);
}

}
