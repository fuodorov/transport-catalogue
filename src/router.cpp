#include "router.h"

#include "log/easylogging++.h"

namespace transport_catalogue::router {

void TransportRouter::SetRoutingSettings(RoutingSettings routing_settings) {
  routing_settings_ = std::move(routing_settings);
}
const RoutingSettings &TransportRouter::GetRoutingSettings() const {
  return routing_settings_;
}

void TransportRouter::BuildRouter(TransportCatalogue &transport_catalogue) {
  SetGraph(transport_catalogue);
  router_ = std::make_unique<Router<double>>(*graph_);
  router_->Build();
}

const DirectedWeightedGraph<double> &TransportRouter::GetGraph() const {
  return *graph_;
}
const Router<double> &TransportRouter::GetRouter() const { return *router_; }
const std::variant<StopEdge, BusEdge> &TransportRouter::GetEdge(
    EdgeId id) const {
  return edge_id_to_edge_.at(id);
}

std::optional<RouterStop> TransportRouter::GetRouterStop(Stop *stop) const {
  if (stop_to_router_.count(stop)) {
    return stop_to_router_.at(stop);
  } else {
    return std::nullopt;
  }
}

std::optional<RouteInfo> TransportRouter::GetRouteInfo(
    VertexId start, graph::VertexId end) const {
  const auto &route_info = router_->BuildRoute(start, end);
  if (route_info) {
    RouteInfo result;
    result.total_time = route_info->weight;

    for (const auto edge : route_info->edges) {
      result.edges.emplace_back(GetEdge(edge));
    }

    return result;

  } else {
    return std::nullopt;
  }
}

const std::unordered_map<Stop *, RouterStop> &TransportRouter::GetStopVertex()
    const {
  return stop_to_router_;
}
const std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>>
    &TransportRouter::GetEdgeId() const {
  return edge_id_to_edge_;
}

std::deque<Stop *> TransportRouter::GetStops(
    TransportCatalogue &transport_catalogue) {
  std::deque<Stop *> stops_ptr;

  for (auto [_, stop_ptr] : transport_catalogue.GetStopNames()) {
    stops_ptr.push_back(stop_ptr);
  }

  return stops_ptr;
}

std::deque<Bus *> TransportRouter::GetBuses(
    TransportCatalogue &transport_catalogue) {
  std::deque<Bus *> buses_ptr;

  for (auto [_, bus_ptr] : transport_catalogue.GetBusNames()) {
    buses_ptr.push_back(bus_ptr);
  }

  return buses_ptr;
}

void TransportRouter::SetStops(const std::deque<Stop *> &stops) {
  size_t i = 0;

  for (const auto stop : stops) {
    VertexId first = i++;
    VertexId second = i++;

    stop_to_router_[stop] = RouterStop{first, second};
  }
}

void TransportRouter::AddEdgeStop() {
  for (const auto [stop, num] : stop_to_router_) {
    EdgeId id = graph_->AddEdge(Edge<double>{
        num.bus_wait_start, num.bus_wait_end, routing_settings_.bus_wait_time});

    edge_id_to_edge_[id] =
        StopEdge{stop->name, routing_settings_.bus_wait_time};
  }
}

void TransportRouter::AddEdgeBus(TransportCatalogue &transport_catalogue) {
  for (auto bus : GetBuses(transport_catalogue)) {
    ParseBus(bus->stops.begin(), bus->stops.end(), transport_catalogue, bus);

    if (!bus->is_round_trip) {
      ParseBus(bus->stops.rbegin(), bus->stops.rend(), transport_catalogue,
               bus);
    }
  }
}

void TransportRouter::SetGraph(TransportCatalogue &transport_catalogue) {
  const auto stops_ptr_size = GetStops(transport_catalogue).size();

  graph_ = std::make_unique<DirectedWeightedGraph<double>>(2 * stops_ptr_size);

  SetStops(GetStops(transport_catalogue));
  AddEdgeStop();
  AddEdgeBus(transport_catalogue);
}

Edge<double> TransportRouter::MakeEdgeBus(Stop *start, Stop *end,
                                          const double distance) const {
  Edge<double> result;

  result.from = stop_to_router_.at(start).bus_wait_end;
  result.to = stop_to_router_.at(end).bus_wait_start;
  result.weight = distance * 1.0 / (routing_settings_.bus_velocity * KM / HR);
  LOG(DEBUG) << "Make edge for bus " << start->name << " to " << end->name
             << " with weight " << result.weight;
  return result;
}

}  // end namespace transport_catalogue::router