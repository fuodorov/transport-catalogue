#include "transport_router.h"

namespace routing {

TransportRouter::TransportRouter(const catalogue::TransportCatalogue &catalogue,
                                 Settings settings)
    : catalogue_(catalogue), settings_(settings) {
  BuildVertexesForStops(catalogue.GetUniqueStops());
  BuildRoutesGraph(catalogue.GetBuses());

  router_ = std::make_unique<Router>(*routes_);
}

void TransportRouter::BuildVertexesForStops(
    const std::set<std::string_view> &stops) {
  graph::VertexId start{0};
  graph::VertexId end{1};

  stop_vertexes_.reserve(stops.size());

  for (std::string_view stop : stops) {
    stop_vertexes_.emplace(stop, StopVertexes{start, end});

    start += 2;
    end += 2;
  }
}

void TransportRouter::AddBusRouteEdges(const catalogue::Bus &bus) {
  for (const auto &[route, info] :
       catalogue_.GetAllDistances(bus.number, settings_.velocity_)) {
    auto edge =
        graph::Edge<Weight>{stop_vertexes_[route.first].end,
                            stop_vertexes_[route.second].start, info.time};
    routes_->AddEdge(edge);
    edge_responses_.emplace(
        edge, BusResponse(info.time, bus.number, info.stops_count));
  }
}

void TransportRouter::BuildRoutesGraph(
    const std::deque<catalogue::Bus> &buses) {
  routes_ = std::make_unique<Graph>(stop_vertexes_.size() * 2);

  auto wait_time = static_cast<double>(settings_.wait_time_);
  auto stop_edge = graph::Edge<Weight>{};

  for (auto [stop_name, stop_vertexes] : stop_vertexes_) {
    stop_edge =
        graph::Edge<Weight>{stop_vertexes.start, stop_vertexes.end, wait_time};

    routes_->AddEdge(stop_edge);
    edge_responses_.emplace(stop_edge, WaitResponse(wait_time, stop_name));
  }

  for (const auto &bus : buses) {
    AddBusRouteEdges(bus);
  }
}

DataResponseOpt TransportRouter::BuildRoute(std::string_view from,
                                            std::string_view to) const {
  DataResponseOpt response{std::nullopt};

  if (auto route = router_->BuildRoute(stop_vertexes_.at(from).start,
                                       stop_vertexes_.at(to).start)) {
    response.emplace(DataResponse{});
    response->total_time = route->weight;

    for (auto edge_id : route->edges) {
      response->items.emplace_back(
          edge_responses_.at(routes_->GetEdge(edge_id)));
    }
  }

  return response;
}

} // namespace routing