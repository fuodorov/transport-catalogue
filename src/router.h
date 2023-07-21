#pragma once

#include <deque>
#include <iostream>
#include <unordered_map>

#include "catalogue.h"
#include "domain.h"
#include "graph/router.h"

namespace transport_catalogue::router {

using namespace domain;
using namespace graph;

static const uint16_t KM = 1000;
static const uint16_t HR = 60;

class TransportRouter {
 public:
  void SetRoutingSettings(RoutingSettings routing_settings);
  const RoutingSettings &GetRoutingSettings() const;

  void BuildRouter(TransportCatalogue &transport_catalogue);

  const DirectedWeightedGraph<double> &GetGraph() const;
  const Router<double> &GetRouter() const;
  const std::variant<StopEdge, BusEdge> &GetEdge(EdgeId id) const;

  std::optional<RouterStop> GetRouterStop(Stop *stop) const;
  std::optional<RouteInfo> GetRouteInfo(VertexId start, VertexId end) const;

  const std::unordered_map<Stop *, RouterStop> &GetStopVertex() const;
  const std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>>
      &GetEdgeId() const;

  std::deque<Stop *> GetStops(TransportCatalogue &transport_catalogue);
  std::deque<Bus *> GetBuses(TransportCatalogue &transport_catalogue);

  void AddEdgeStop();
  void AddEdgeBus(TransportCatalogue &transport_catalogue);

  void SetStops(const std::deque<Stop *> &stops);
  void SetGraph(TransportCatalogue &transport_catalogue);

  Edge<double> MakeEdgeBus(Stop *start, Stop *end,
                                const double distance) const;

  template <typename Iterator>
  void ParseBus(Iterator first, Iterator last,
                          const TransportCatalogue &transport_catalogue,
                          const Bus *bus);

 private:
  std::unordered_map<Stop *, RouterStop> stop_to_router_;
  std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> edge_id_to_edge_;

  std::unique_ptr<DirectedWeightedGraph<double>> graph_;
  std::unique_ptr<Router<double>> router_;

  RoutingSettings routing_settings_;
};

template <typename Iterator>
void TransportRouter::ParseBus(
    Iterator first, Iterator last,
    const TransportCatalogue &transport_catalogue, const Bus *bus) {
  for (auto it = first; it != last; ++it) {
    size_t distance = 0;
    size_t span = 0;

    for (auto it2 = std::next(it); it2 != last; ++it2) {
      distance += transport_catalogue.GetDistanceStops(*prev(it2), *it2);
      ++span;

      EdgeId id = graph_->AddEdge(MakeEdgeBus(*it, *it2, distance));

      edge_id_to_edge_[id] =
          BusEdge{bus->name, span, graph_->GetEdge(id).weight};
    }
  }
}

}  // end namespace transport_catalogue::router