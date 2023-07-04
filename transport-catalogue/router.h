#pragma once

#include <variant>

#include "catalogue.h"
#include "graph/router.h"

namespace routing {

struct Settings {
  double speed_{0};
  int time_wait_{0};
};

struct WaitResponse {
  double time{0.};
  std::string type{"Wait"}, stop_name;

  WaitResponse(double time, std::string_view stop)
      : time(time), stop_name(stop) {}
};

struct BusResponse {
  double time{0.};
  std::string type{"Bus"}, bus;
  int span_count{0};

  BusResponse(double time, const std::string &bus, int stops_count)
      : time(time), bus(bus), span_count(stops_count) {}
};

using ItemResponse = std::variant<WaitResponse, BusResponse>;

struct DataResponse {
  double total_time{0.};
  std::vector<ItemResponse> items;
};

using DataResponseOpt = std::optional<DataResponse>;

class TransportRouter {
public:
  using Weight = double;
  using Graph = graph::DirectedWeightedGraph<Weight>;
  using Router = graph::Router<Weight>;

  TransportRouter(const catalogue::TransportCatalogue &catalogue,
                  Settings settings);

  [[nodiscard]] DataResponseOpt BuildRoute(std::string_view from,
                                           std::string_view to) const;

private:
  void BuildVertexes(const std::set<std::string_view> &stops);
  void AddBus(const catalogue::Bus &bus);
  void BuildGraph(const std::deque<catalogue::Bus> &buses);

  struct StopVertexes {
    graph::VertexId start{0};
    graph::VertexId end{0};
  };

  struct EdgeHash {
    inline size_t operator()(const graph::Edge<Weight> &edge) const {
      return even_ * std::hash<size_t>{}(edge.from) +
             even_ * even_ * std::hash<size_t>{}(edge.to) +
             even_ * even_ * even_ * std::hash<Weight>{}(edge.weight);
    }

  private:
    static constexpr size_t even_{42};
  };

  const catalogue::TransportCatalogue &catalogue_;
  Settings settings_;

  std::unordered_map<std::string_view, StopVertexes> stop_vertexes_;
  std::unordered_map<graph::Edge<Weight>, ItemResponse, EdgeHash>
      edge_responses_;

  std::unique_ptr<Graph> routes_{nullptr};
  std::unique_ptr<Router> router_{nullptr};
};

using TransportRouterOpt = std::optional<TransportRouter>;

} // namespace routing