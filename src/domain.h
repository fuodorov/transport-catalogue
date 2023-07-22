#pragma once

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

#include "geo/geo.h"
#include "graph/graph.h"

namespace domain {

struct RoutingSettings;
struct RouteInfo;
struct BusInfo;
struct StopInfo;
struct StatisticRequest;
struct Bus;
struct Stop;
struct Distance;
struct RouterStop;
struct StopEdge;
struct BusEdge;

struct StatisticRequest {
  int id;
  std::string type;
  std::string name;
  std::string from;
  std::string to;
};

struct Stop {
  std::string name;
  double latitude;
  double longitude;
  std::vector<Bus *> buses;
};

struct Bus {
  std::string name;
  std::vector<Stop *> stops;
  bool is_round_trip;
  size_t route_length;
};

struct Distance {
  const Stop *start;
  const Stop *end;
  int distance;
};

struct BusInfo {
  std::string_view name;
  bool not_found;
  int stops_on_route;
  int unique_stops;
  int route_length;
  double curvature;
};

struct StopInfo {
  std::string_view name;
  bool not_found;
  std::vector<std::string> buses_name;
};

struct StopEdge {
  std::string_view name;
  double time = 0;
};

struct BusEdge {
  std::string_view name;
  size_t span_count = 0;
  double time = 0;
};

struct RoutingSettings {
  double bus_wait_time = 0;
  double bus_velocity = 0;
};

struct RouterStop {
  graph::VertexId bus_wait_start;
  graph::VertexId bus_wait_end;
};

struct RouteInfo {
  double total_time = 0.;
  std::vector<std::variant<StopEdge, BusEdge>> edges;
};

}  // end namespace domain