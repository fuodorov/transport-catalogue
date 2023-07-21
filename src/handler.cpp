#include "handler.h"

namespace handler {

struct EdgeInfoGetter {
  Node operator()(const StopEdge &edge_info) {
    using namespace std::literals;

    return Builder{}
        .StartDict()
        .Key("type")
        .Value("Wait")
        .Key("stop_name")
        .Value(std::string(edge_info.name))
        .Key("time")
        .Value(edge_info.time)
        .EndDict()
        .Build();
  }

  Node operator()(const BusEdge &edge_info) {
    using namespace std::literals;

    return Builder{}
        .StartDict()
        .Key("type")
        .Value("Bus")
        .Key("bus")
        .Value(std::string(edge_info.bus_name))
        .Key("span_count")
        .Value(static_cast<int>(edge_info.span_count))
        .Key("time")
        .Value(edge_info.time)
        .EndDict()
        .Build();
  }
};

Node Handler::MakeStopNode(int id_request, const StopInfo &stop_info) {
  Node result;
  Array buses;
  Builder builder;

  std::string str_not_found = "not found";

  if (stop_info.not_found) {
    builder.StartDict()
        .Key("request_id")
        .Value(id_request)
        .Key("error_message")
        .Value(str_not_found)
        .EndDict();

    result = builder.Build();

  } else {
    builder.StartDict()
        .Key("request_id")
        .Value(id_request)
        .Key("buses")
        .StartArray();

    for (std::string bus_name : stop_info.buses_name) {
      builder.Value(bus_name);
    }

    builder.EndArray().EndDict();

    result = builder.Build();
  }

  return result;
}

Node Handler::MakeBusNode(int id_request, const BusInfo &bus_info) {
  Node result;
  std::string str_not_found = "not found";

  if (bus_info.not_found) {
    result = Builder{}
                 .StartDict()
                 .Key("request_id")
                 .Value(id_request)
                 .Key("error_message")
                 .Value(str_not_found)
                 .EndDict()
                 .Build();
  } else {
    result = Builder{}
                 .StartDict()
                 .Key("request_id")
                 .Value(id_request)
                 .Key("curvature")
                 .Value(bus_info.curvature)
                 .Key("route_length")
                 .Value(bus_info.route_length)
                 .Key("stop_count")
                 .Value(bus_info.stops_on_route)
                 .Key("unique_stop_count")
                 .Value(bus_info.unique_stops)
                 .EndDict()
                 .Build();
  }

  return result;
}

Node Handler::MakeMapNode(int id_request, TransportCatalogue &catalogue_,
                          RenderSettings render_settings) {
  Node result;

  std::ostringstream map_stream;
  std::string map_str;

  MapRenderer map_catalogue(render_settings);

  map_catalogue.InitSphereProjector(GetStopsCoordinates(catalogue_));

  RenderMap(map_catalogue, catalogue_);
  map_catalogue.GetStreamMap(map_stream);
  map_str = map_stream.str();

  result = Builder{}
               .StartDict()
               .Key("request_id")
               .Value(id_request)
               .Key("map")
               .Value(map_str)
               .EndDict()
               .Build();

  return result;
}

Node Handler::MakeRouteNode(StatisticRequest &request,
                            TransportCatalogue &catalogue,
                            TransportRouter &routing) {
  const auto &route_info =
      GetRouteInfo(request.from, request.to, catalogue, routing);

  if (!route_info) {
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
  for (const auto &item : route_info->edges) {
    items.emplace_back(std::visit(EdgeInfoGetter{}, item));
  }

  return Builder{}
      .StartDict()
      .Key("request_id")
      .Value(request.id)
      .Key("total_time")
      .Value(route_info->total_time)
      .Key("items")
      .Value(items)
      .EndDict()
      .Build();
}

void Handler::Queries(TransportCatalogue &catalogue,
                      std::vector<StatisticRequest> &stat_requests,
                      RenderSettings &render_settings,
                      RoutingSettings &routing_settings) {
  std::vector<Node> result_request;
  TransportRouter transport_router;

  transport_router.set_routing_settings(routing_settings);
  transport_router.build_router(catalogue);

  for (StatisticRequest req : stat_requests) {
    if (req.type == "Stop") {
      result_request.push_back(
          MakeStopNode(req.id, StopQuery(catalogue, req.name)));

    } else if (req.type == "Bus") {
      result_request.push_back(
          MakeBusNode(req.id, BusQuery(catalogue, req.name)));

    } else if (req.type == "Map") {
      result_request.push_back(MakeMapNode(req.id, catalogue, render_settings));

    } else if (req.type == "Route") {
      result_request.push_back(MakeRouteNode(req, catalogue, transport_router));
    }
  }

  document = Document{Node(result_request)};
}

void Handler::RenderMap(MapRenderer &map_catalogue,
                        TransportCatalogue &catalogue) const {
  std::vector<std::pair<Bus *, int>> buses_palette;
  std::vector<Stop *> stops_sort;
  int palette_size = 0;
  int palette_index = 0;

  palette_size = map_catalogue.GetPaletteSize();

  if (palette_size == 0) {
    std::cout << "color palette is empty";
    return;
  }

  auto buses = catalogue.GetBusNames();
  if (buses.size() > 0) {
    for (std::string_view bus_name : GetBusNames(catalogue)) {
      Bus *bus_info = catalogue.GetBus(bus_name);

      if (bus_info) {
        if (bus_info->stops.size() > 0) {
          buses_palette.push_back(std::make_pair(bus_info, palette_index));
          palette_index++;

          if (palette_index == palette_size) {
            palette_index = 0;
          }
        }
      }
    }

    if (buses_palette.size() > 0) {
      map_catalogue.AddLine(buses_palette);
      map_catalogue.AddBusesName(buses_palette);
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
        stops_sort.push_back(stop);
      }
    }

    if (stops_sort.size() > 0) {
      map_catalogue.AddStopsCircle(stops_sort);
      map_catalogue.AddStopsName(stops_sort);
    }
  }
}

std::optional<RouteInfo> Handler::GetRouteInfo(std::string_view start,
                                               std::string_view end,
                                               TransportCatalogue &catalogue,
                                               TransportRouter &routing) const {
  return routing.GetRouteInfo(
      routing.get_router_by_stop(catalogue.GetStop(start))->bus_wait_start,
      routing.get_router_by_stop(catalogue.GetStop(end))->bus_wait_start);
}

std::vector<geo::Coordinates> Handler::GetStopsCoordinates(
    TransportCatalogue &catalogue_) const {
  std::vector<geo::Coordinates> stops_coordinates;
  auto buses = catalogue_.GetBusNames();

  for (auto &[busname, bus] : buses) {
    for (auto &stop : bus->stops) {
      geo::Coordinates coordinates;
      coordinates.latitude = stop->latitude;
      coordinates.longitude = stop->longitude;

      stops_coordinates.push_back(coordinates);
    }
  }
  return stops_coordinates;
}

std::vector<std::string_view> Handler::GetBusNames(
    TransportCatalogue &catalogue_) const {
  std::vector<std::string_view> buses_names;

  auto buses = catalogue_.GetBusNames();
  if (buses.size() > 0) {
    for (auto &[busname, bus] : buses) {
      buses_names.push_back(busname);
    }

    std::sort(buses_names.begin(), buses_names.end());

    return buses_names;

  } else {
    return {};
  }
}

BusInfo Handler::BusQuery(TransportCatalogue &catalogue,
                          std::string_view bus_name) {
  BusInfo bus_info;
  Bus *bus = catalogue.GetBus(bus_name);

  if (bus != nullptr) {
    bus_info.name = bus->name;
    bus_info.not_found = false;
    bus_info.stops_on_route = static_cast<int>(bus->stops.size());
    bus_info.unique_stops =
        static_cast<int>(catalogue.GetUniqueStops(bus).size());
    bus_info.route_length = static_cast<int>(bus->route_length);
    bus_info.curvature =
        double(catalogue.GetDistanceBuses(bus) / catalogue.GetLength(bus));
  } else {
    bus_info.name = bus_name;
    bus_info.not_found = true;
  }

  return bus_info;
}

StopInfo Handler::StopQuery(TransportCatalogue &catalogue,
                            std::string_view stop_name) {
  std::unordered_set<const Bus *> unique_buses;
  StopInfo stop_info;
  Stop *stop = catalogue.GetStop(stop_name);

  if (stop != NULL) {
    stop_info.name = stop->name;
    stop_info.not_found = false;
    unique_buses = catalogue.GetUniqueBuses(stop);

    if (unique_buses.size() > 0) {
      for (const Bus *bus : unique_buses) {
        stop_info.buses_name.push_back(bus->name);
      }

      std::sort(stop_info.buses_name.begin(), stop_info.buses_name.end());
    }

  } else {
    stop_info.name = stop_name;
    stop_info.not_found = true;
  }

  return stop_info;
}

const Document &Handler::GetDocument() { return document; }

}  // end namespace handler