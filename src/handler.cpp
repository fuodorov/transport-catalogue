#include "handler.h"

namespace handler {

struct Edge {
  Node operator()(const StopEdge &edge) {
    using namespace std::literals;

    return Builder{}
        .StartDict()
        .Key("type")
        .Value("Wait")
        .Key("stop_name")
        .Value(std::string(edge.name))
        .Key("time")
        .Value(edge.time)
        .EndDict()
        .Build();
  }

  Node operator()(const BusEdge &edge) {
    using namespace std::literals;

    return Builder{}
        .StartDict()
        .Key("type")
        .Value("Bus")
        .Key("bus")
        .Value(std::string(edge.name))
        .Key("span_count")
        .Value(static_cast<int>(edge.span_count))
        .Key("time")
        .Value(edge.time)
        .EndDict()
        .Build();
  }
};

Node Handler::MakeStopNode(int request_id, const StopInfo &stop) {
  if (stop.not_found) {
    return Builder{}
        .StartDict()
        .Key("request_id")
        .Value(request_id)
        .Key("error_message")
        .Value("not found")
        .EndDict()
        .Build();
  } else {
    Array buses;
    for (std::string name : stop.buses_name) {
      buses.emplace_back(name);
    }

    return Builder{}
        .StartDict()
        .Key("request_id")
        .Value(request_id)
        .Key("buses")
        .Value(buses)
        .EndDict()
        .Build();
  }
}

Node Handler::MakeBusNode(int request_id, const BusInfo &bus_info) {
  return bus_info.not_found ? Builder{}
                                  .StartDict()
                                  .Key("request_id")
                                  .Value(request_id)
                                  .Key("error_message")
                                  .Value("not found")
                                  .EndDict()
                                  .Build()
                            : Builder{}
                                  .StartDict()
                                  .Key("request_id")
                                  .Value(request_id)
                                  .Key("stop_count")
                                  .Value(bus_info.stops_on_route)
                                  .Key("unique_stop_count")
                                  .Value(bus_info.unique_stops)
                                  .Key("route_length")
                                  .Value(bus_info.route_length)
                                  .Key("curvature")
                                  .Value(bus_info.curvature)
                                  .EndDict()
                                  .Build();
}

Node Handler::MakeMapNode(int request_id, TransportCatalogue &catalogue_,
                          RenderSettings render_settings) {
  std::ostringstream map_stream;
  std::string map_str;

  MapRenderer map_catalogue(render_settings);

  map_catalogue.InitSphereProjector(GetStopsCoordinates(catalogue_));

  RenderMap(map_catalogue, catalogue_);
  map_catalogue.GetStreamMap(map_stream);
  map_str = map_stream.str();

  return Builder{}
      .StartDict()
      .Key("request_id")
      .Value(request_id)
      .Key("map")
      .Value(map_str)
      .EndDict()
      .Build();
}

Node Handler::MakeRouteNode(StatisticRequest &request,
                            TransportCatalogue &catalogue,
                            TransportRouter &routing) {
  const auto &route =
      GetRouteInfo(request.from, request.to, catalogue, routing);

  if (!route) {
    return Builder{}
        .StartDict()
        .Key("request_id")
        .Value(request.id)
        .Key("error_message")
        .Value("not found")
        .EndDict()
        .Build();
  }

  Array items;
  for (const auto &item : route->edges) {
    items.emplace_back(std::visit(Edge{}, item));
  }

  return Builder{}
      .StartDict()
      .Key("request_id")
      .Value(request.id)
      .Key("total_time")
      .Value(route->total_time)
      .Key("items")
      .Value(items)
      .EndDict()
      .Build();
}

void Handler::Queries(TransportCatalogue &catalogue,
                      std::vector<StatisticRequest> &stat_requests,
                      RenderSettings &render_settings,
                      RoutingSettings &routing_settings) {
  std::vector<Node> result;
  TransportRouter router;

  router.SetRoutingSettings(routing_settings);
  router.BuildRouter(catalogue);

  for (StatisticRequest req : stat_requests) {
    if (req.type == "Stop") {
      result.push_back(MakeStopNode(req.id, StopQuery(catalogue, req.name)));

    } else if (req.type == "Bus") {
      result.push_back(MakeBusNode(req.id, BusQuery(catalogue, req.name)));

    } else if (req.type == "Map") {
      result.push_back(MakeMapNode(req.id, catalogue, render_settings));

    } else if (req.type == "Route") {
      result.push_back(MakeRouteNode(req, catalogue, router));
    }
  }

  document = Document{Node(result)};
}

void Handler::RenderMap(MapRenderer &map_catalogue,
                        TransportCatalogue &catalogue) const {
  std::vector<std::pair<Bus *, int>> palettes;
  std::vector<Stop *> tmp_stops;
  int size = map_catalogue.GetPaletteSize();
  int id = 0;

  if (size == 0) {
    std::cout << "color palette is empty";
    return;
  }

  auto buses = catalogue.GetBusNames();
  if (buses.size() > 0) {
    for (std::string_view name : GetBusNames(catalogue)) {
      Bus *bus = catalogue.GetBus(name);

      if (bus && bus->stops.size() > 0) {
        palettes.push_back(std::make_pair(bus, id));
        id++;

        if (id == size) {
          id = 0;
        }
      }
    }

    if (palettes.size() > 0) {
      map_catalogue.AddLine(palettes);
      map_catalogue.AddBusesName(palettes);
    }
  }

  auto stops = catalogue.GetStopNames();
  if (stops.size() > 0) {
    std::vector<std::string_view> stops_name;

    for (auto &[stop_name, stop] : stops) {
      if (stop->buses.size() > 0) {
        stops_name.push_back(stop_name);
      }
    }

    std::sort(stops_name.begin(), stops_name.end());

    for (std::string_view stop_name : stops_name) {
      Stop *stop = catalogue.GetStop(stop_name);
      if (stop) {
        tmp_stops.push_back(stop);
      }
    }

    if (tmp_stops.size() > 0) {
      map_catalogue.AddStopsCircle(tmp_stops);
      map_catalogue.AddStopsName(tmp_stops);
    }
  }
}

std::optional<RouteInfo> Handler::GetRouteInfo(std::string_view start,
                                               std::string_view end,
                                               TransportCatalogue &catalogue,
                                               TransportRouter &routing) const {
  return routing.GetRouteInfo(
      routing.GetRouterStop(catalogue.GetStop(start))->bus_wait_start,
      routing.GetRouterStop(catalogue.GetStop(end))->bus_wait_start);
}

std::vector<geo::Coordinates> Handler::GetStopsCoordinates(
    TransportCatalogue &catalogue_) const {
  std::vector<geo::Coordinates> coordinates;

  for (auto &[busname, bus] : catalogue_.GetBusNames()) {
    for (auto &stop : bus->stops) {
      coordinates.push_back({stop->latitude, stop->longitude});
    }
  }
  return coordinates;
}

std::vector<std::string_view> Handler::GetBusNames(
    TransportCatalogue &catalogue_) const {
  std::vector<std::string_view> names;
  names.reserve(catalogue_.GetBuses().size());
  for (const auto &bus : catalogue_.GetBuses()) {
    names.push_back(bus.name);
  }
  std::sort(names.begin(), names.end());
  return names;
}

BusInfo Handler::BusQuery(TransportCatalogue &catalogue,
                          std::string_view name) {
  BusInfo info;
  Bus *bus = catalogue.GetBus(name);

  if (bus != nullptr) {
    info.name = bus->name;
    info.not_found = false;
    info.stops_on_route = static_cast<int>(bus->stops.size());
    info.unique_stops = static_cast<int>(catalogue.GetUniqueStops(bus).size());
    info.route_length = static_cast<int>(bus->route_length);
    info.curvature =
        double(catalogue.GetDistanceBuses(bus) / catalogue.GetLength(bus));
  } else {
    info.name = name;
    info.not_found = true;
  }

  return info;
}

StopInfo Handler::StopQuery(TransportCatalogue &catalogue,
                            std::string_view name) {
  std::unordered_set<const Bus *> unique;
  StopInfo info;
  Stop *stop = catalogue.GetStop(name);

  if (stop != nullptr) {
    info.name = stop->name;
    info.not_found = false;
    unique = catalogue.GetUniqueBuses(stop);

    if (unique.size() > 0) {
      for (const Bus *bus : unique) {
        info.buses_name.push_back(bus->name);
      }

      std::sort(info.buses_name.begin(), info.buses_name.end());
    }

  } else {
    info.name = name;
    info.not_found = true;
  }

  return info;
}

const Document &Handler::GetDocument() { return document; }

}  // end namespace handler