#pragma once

#include <cube/gfx/gradient.hpp>
#include <cube/core/json_util.hpp>

namespace cube::gfx
{

inline void to_json(gradient_stop const & stop, nlohmann::json & out)
{
    out["stop_position"] = stop.gpos;
    core::to_json(stop.gcolor, out["stop_color"]);
}

inline void from_json(nlohmann::json const & json, gradient_stop & out)
{
    using std::operator""s;

    if (!json.is_object())
        throw std::invalid_argument("Expected JSON object for gradient_stop, got: "s + json.type_name());

    out.gpos = core::parse_field<double>(json, "stop_position");
    core::from_json(json["stop_color"], out.gcolor);
}

inline void to_json(gradient const & g, nlohmann::json & out)
{
    core::to_json(g.stops(), out["gradient_stops"]);
}

inline void from_json(nlohmann::json const & json, gradient & out)
{
    using std::operator""s;

    if (!json.is_object())
        throw std::invalid_argument("Expected JSON object for gradient, got: "s + json.type_name());

    std::vector<gradient_stop> stops;
    core::from_json(json["gradient_stops"], stops);

    for (auto const & stop : stops)
        out.add(stop);
}

}
