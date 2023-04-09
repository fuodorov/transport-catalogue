#include "parser.h"

#include <string>

namespace parser {
    using namespace catalogue;
    using namespace parser;
    using namespace std::literals;

    namespace {
        std::pair<catalogue::Stop, bool> ParseBusStopInput(const json::Dict& info) {
            Stop stop;
            stop.name = info.at("name"s).AsString();
            stop.point.lat = info.at("latitude"s).AsDouble();
            stop.point.lng = info.at("longitude"s).AsDouble();
            return {std::move(stop), !info.at("road_distances"s).AsMap().empty()};
        }

        Bus ParseBusRouteInput(const json::Dict& info) {
            Bus bus;
            bus.number = info.at("name"s).AsString();
            bus.type = info.at("is_roundtrip"s).AsBool() ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;

            const auto& stops = info.at("stops"s).AsArray();
            bus.stops.reserve(stops.size());

            for (const auto& stop : stops)
                bus.stops.emplace_back(stop.AsString());

            bus.unique_stops = {bus.stops.begin(), bus.stops.end()};

            return bus;
        }

        renderer::Screen ParseScreenSettings(const json::Dict& settings) {
            return renderer::Screen{settings.at("width"s).AsDouble(), settings.at("height"s).AsDouble(), settings.at("padding"s).AsDouble()};
        }

        renderer::Label ParseLabelSettings(const json::Dict& settings, const std::string& key_type) {
            const json::Array offset = settings.at(key_type + "_label_offset"s).AsArray();
            return {settings.at(key_type + "_label_font_size"s).AsInt(), {offset.at(0).AsDouble(), offset.at(1).AsDouble()}};
        }

        svg::Color ParseColor(const json::Node& node) {
            if (node.IsString()){
                return svg::Color(node.AsString());
            }

            const auto& array = node.AsArray();

            if (array.size() == 3) {
                return svg::Rgb(array.at(0).AsInt(), array.at(1).AsInt(), array.at(2).AsInt());
            } else {
                return svg::Rgba(array.at(0).AsInt(), array.at(1).AsInt(), array.at(2).AsInt(), array.at(3).AsDouble());
            }

        }

        renderer::UnderLayer ParseLayer(const json::Dict& settings) {
            return renderer::UnderLayer{ParseColor(settings.at("underlayer_color"s)), settings.at("underlayer_width"s).AsDouble()};
        }

    }  // namespace

    TransportCatalogue ParseQueries(const json::Array& requests) {
        TransportCatalogue catalogue;

        std::vector<int> requests_ids_with_road_distances;
        requests_ids_with_road_distances.reserve(requests.size());

        std::vector<int> requests_ids_with_buses;
        requests_ids_with_buses.reserve(requests.size());

        for (int id = 0; id != static_cast<int>(requests.size()); ++id) {
            const auto& request_dict_view = requests.at(id).AsMap();

            if (request_dict_view.at("type"s) == "Stop"s) {
                auto [stop, has_road_distances] = ParseBusStopInput(request_dict_view);
                if (has_road_distances)
                    requests_ids_with_road_distances.emplace_back(id);

                catalogue.AddStop(std::move(stop));
            } else if (request_dict_view.at("type"s) == "Bus"s) {
                requests_ids_with_buses.emplace_back(id);
            }
        }

        for (int id : requests_ids_with_road_distances) {
            const auto& request_dict_view = requests.at(id).AsMap();

            std::string_view stop_from = request_dict_view.at("name"s).AsString();
            for (const auto& [stop_to, distance] : request_dict_view.at("road_distances"s).AsMap())
                catalogue.AddDistance(stop_from, stop_to, distance.AsInt());
        }

        for (int id : requests_ids_with_buses) {
            const auto& request_dict_view = requests.at(id).AsMap();
            catalogue.AddBus(ParseBusRouteInput(request_dict_view));
        }

        return catalogue;
    }

    renderer::Visualization ParseRenderSettings(const json::Dict& settings) {
        renderer::Visualization final_settings;

        double line_width = settings.at("line_width"s).AsDouble();
        double stop_radius = settings.at("stop_radius"s).AsDouble();
        const auto& colors = settings.at("color_palette"s).AsArray();
        std::vector<svg::Color> svg_colors;
        svg_colors.reserve(colors.size());

        for (const auto& color : colors) {
            svg_colors.emplace_back(ParseColor(color));
        }

        final_settings.SetScreen(ParseScreenSettings(settings))
            .SetLineWidth(line_width)
            .SetStopRadius(stop_radius)
            .SetLabels(renderer::LabelType::Stop, ParseLabelSettings(settings, "stop"s))
            .SetLabels(renderer::LabelType::Bus, ParseLabelSettings(settings, "bus"s))
            .SetUnderLayer(ParseLayer(settings))
            .SetColors(std::move(svg_colors));

        return final_settings;
    }

}  // namespace parser