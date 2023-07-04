#include "json_reader.h"

#include <string>

#include "json_builder.h"

namespace request {

using namespace std::literals;
using namespace catalogue;
using namespace request;
using namespace routing;

namespace {

std::pair<catalogue::Stop, bool> ParseStop(const json::Dict &info) {
  Stop stop;

  stop.name = info.at("name"s).AsString();
  stop.point.lat = info.at("latitude"s).AsDouble();
  stop.point.lng = info.at("longitude"s).AsDouble();

  return {std::move(stop), !info.at("road_distances"s).AsDict().empty()};
}

Bus ParseRoute(const json::Dict &info) {
  Bus bus;

  const auto &stops = info.at("stops"s).AsArray();
  bus.stops.reserve(stops.size());

  for (const auto &stop : stops) {
    bus.stops.emplace_back(stop.AsString());
  }

  bus.unique_stops = {bus.stops.begin(), bus.stops.end()};

  bus.number = info.at("name"s).AsString();
  bus.type = info.at("is_roundtrip"s).AsBool() ? RouteType::CIRCLE
                                               : RouteType::TWO_DIRECTIONAL;

  return bus;
}

void MakeBusResponse(int request_id, const BusStatistics &statistics,
                     json::Builder &response) {

  response.StartDict()
      .Key("curvature"s)
      .Value(statistics.curvature)
      .Key("request_id"s)
      .Value(request_id)
      .Key("route_length"s)
      .Value(statistics.rout_length)
      .Key("stop_count"s)
      .Value(static_cast<int>(statistics.stops_count))
      .Key("unique_stop_count"s)
      .Value(static_cast<int>(statistics.unique_stops_count))
      .EndDict();
}

void MakeStopResponse(int request_id, const std::set<std::string_view> &buses,
                      json::Builder &response) {
  response.StartDict().Key("request_id"s).Value(request_id);

  response.Key("buses"s).StartArray();
  for (std::string_view bus : buses) {
    response.Value(std::string(bus));
  }
  response.EndArray().EndDict();
}

struct RouteItemVisitor {
  json::Builder &json;

  void operator()(const WaitResponse &response) const {
    json.Key("type"s)
        .Value(response.type)
        .Key("stop_name"s)
        .Value(response.stop_name)
        .Key("time"s)
        .Value(response.time);
  }

  void operator()(const BusResponse &response) const {
    json.Key("type"s)
        .Value(response.type)
        .Key("bus")
        .Value(response.bus)
        .Key("span_count"s)
        .Value(response.span_count)
        .Key("time"s)
        .Value(response.time);
  }
};

void MakeRouteResponse(int request_id, const routing::ResponseData &route_info,
                       json::Builder &response) {
  response.StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("total_time"s)
      .Value(route_info.total_time);

  response.Key("items"s).StartArray();
  for (const auto &item : route_info.items) {
    response.StartDict();
    std::visit(RouteItemVisitor{response}, item);
    response.EndDict();
  }

  response.EndArray().EndDict();
}

void MakeErrorResponse(int request_id, json::Builder &response) {
  response.StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("error_message"s)
      .Value("not found"s)
      .EndDict();
}

void MakeMapImageResponse(int request_id, const std::string &image,
                          json::Builder &response) {
  response.StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("map"s)
      .Value(image)
      .EndDict();
}

render::Screen ParseScreenSettings(const json::Dict &settings) {
  return {settings.at("width"s).AsDouble(), settings.at("height"s).AsDouble(),
          settings.at("padding"s).AsDouble()};
}

render::Label ParseLabelSettings(const json::Dict &settings,
                                 const std::string &key_type) {
  const json::Array offset = settings.at(key_type + "_label_offset"s).AsArray();

  return {settings.at(key_type + "_label_font_size"s).AsInt(),
          {offset.at(0).AsDouble(), offset.at(1).AsDouble()}};
}

svg::Color ParseColor(const json::Node &node) {
  if (node.IsString()) {
    return node.AsString();
  }

  const auto &array = node.AsArray();
  uint8_t red = array.at(0).AsInt();
  uint8_t green = array.at(1).AsInt();
  uint8_t blue = array.at(2).AsInt();

  if (array.size() == 3) {
    return svg::Rgb(red, green, blue);
  }

  double alpha = array.at(3).AsDouble();
  return svg::Rgba(red, green, blue, alpha);
}

render::UnderLayer ParseLayer(const json::Dict &settings) {
  return {ParseColor(settings.at("underlayer_color"s)),
          settings.at("underlayer_width"s).AsDouble()};
}

} // namespace

TransportCatalogue ParseBaseRequest(const json::Array &requests) {
  TransportCatalogue catalogue;

  std::vector<int> requests_ids_with_road_distances;
  requests_ids_with_road_distances.reserve(requests.size());

  std::vector<int> requests_ids_with_buses;
  requests_ids_with_buses.reserve(requests.size());

  for (int id = 0; id != static_cast<int>(requests.size()); ++id) {
    const auto &request_ = requests.at(id).AsDict();

    if (request_.at("type"s) == "Stop"s) {
      auto [stop, has_road_distances] = ParseStop(request_);
      if (has_road_distances)
        requests_ids_with_road_distances.emplace_back(id);

      catalogue.AddStop(std::move(stop));
    } else if (request_.at("type"s) == "Bus"s) {
      requests_ids_with_buses.emplace_back(id);
    }
  }

  for (int id : requests_ids_with_road_distances) {
    const auto &request_ = requests.at(id).AsDict();

    std::string_view stop_from = request_.at("name"s).AsString();
    for (const auto &[stop_to, distance] :
         request_.at("road_distances"s).AsDict()) {
      catalogue.AddDistance(stop_from, stop_to, distance.AsInt());
    }
  }

  for (int id : requests_ids_with_buses) {
    const auto &request_ = requests.at(id).AsDict();
    catalogue.AddBus(ParseRoute(request_));
  }

  return catalogue;
}

render::Visualization ParseRenderSettings(const json::Dict &settings) {
  render::Visualization final_settings;

  const auto &colors = settings.at("color_palette"s).AsArray();
  std::vector<svg::Color> svg_colors;
  svg_colors.reserve(colors.size());
  for (const auto &color : colors) {
    svg_colors.emplace_back(ParseColor(color));
  }

  return final_settings.SetScreen(ParseScreenSettings(settings))
      .SetLineWidth(settings.at("line_width"s).AsDouble())
      .SetStopRadius(settings.at("stop_radius"s).AsDouble())
      .SetLabels(render::LabelType::Stop, ParseLabelSettings(settings, "stop"s))
      .SetLabels(render::LabelType::Bus, ParseLabelSettings(settings, "bus"s))
      .SetUnderLayer(ParseLayer(settings))
      .SetColors(std::move(svg_colors));
}

json::Node MakeResponse(const TransportCatalogue &catalogue,
                        TransportRouterOpt &router, const json::Array &requests,
                        const ResponseSettings &settings) {
  auto response = json::Builder();
  response.StartArray();

  for (const auto &request : requests) {
    const auto &request_ = request.AsDict();

    int id = request_.at("id"s).AsInt();
    std::string type = request_.at("type"s).AsString();
    std::string name;

    if (type == "Bus"s) {
      name = request_.at("name"s).AsString();

      if (auto bus_statistics = catalogue.GetBusStatistics(name)) {
        MakeBusResponse(id, *bus_statistics, response);
      } else {
        MakeErrorResponse(id, response);
      }
    } else if (type == "Stop"s) {
      name = request_.at("name"s).AsString();
      if (auto buses = catalogue.GetBusesPassingThroughTheStop(name)) {
        MakeStopResponse(id, *buses, response);
      } else {
        MakeErrorResponse(id, response);
      }
    } else if (type == "Map"s) {
      MakeMapImageResponse(
          id, RenderTransportMap(catalogue, settings.visualization), response);
    } else if (type == "Route"s) {
      if (!router.has_value()) {
        router.emplace(TransportRouter(catalogue, settings.routing));
      }

      if (auto route_info = router->BuildRoute(request_.at("from"s).AsString(),
                                               request_.at("to"s).AsString())) {
        MakeRouteResponse(id, *route_info, response);
      } else {
        MakeErrorResponse(id, response);
      }
    }
  }

  response.EndArray();
  return std::move(response.Build());
}

routing::Settings ParseRoutingSettings(const json::Dict &requests) {
  using namespace routing;

  auto meter_per_min = [](double km_per_hour) {
    return 1'000. * km_per_hour / 60.;
  };

  Settings settings{meter_per_min(requests.at("bus_velocity"s).AsDouble()),
                    requests.at("bus_wait_time"s).AsInt()};

  return settings;
}

void RequestHandler(std::istream &in, std::ostream &out) {
  TransportCatalogue catalogue;
  TransportRouterOpt router{std::nullopt};
  ResponseSettings settings;

  const auto json_ = json::Load(in).GetRoot().AsDict();

  auto transport_catalogue =
      ParseBaseRequest(json_.at("base_requests"s).AsArray());

  settings.visualization =
      ParseRenderSettings(json_.at("render_settings"s).AsDict());

  settings.routing =
      ParseRoutingSettings(json_.at("routing_settings"s).AsDict());

  const auto &stat_requests = json_.at("stat_requests"s).AsArray();

  auto response =
      MakeResponse(transport_catalogue, router, stat_requests, settings);

  json::Print(json::Document{std::move(response)}, out);
}
} // namespace request