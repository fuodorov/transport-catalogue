#pragma once

#include <cstdlib>
#include <vector>

#include "ranges.h"

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
  VertexId from;
  VertexId to;
  Weight weight;
};

template <class Weight>
bool operator==(const Edge<Weight> &left, const Edge<Weight> &right) {
  return std::make_tuple(left.from, left.to, left.weight) ==
         std::make_tuple(right.from, right.to, right.weight);
}

template <typename Weight>
class DirectedWeightedGraph {
 private:
  using Incidence = std::vector<EdgeId>;
  using IncidentEdges = ranges::Range<typename Incidence::const_iterator>;

 public:
  DirectedWeightedGraph() = default;
  explicit DirectedWeightedGraph(size_t vertex_count);
  EdgeId AddEdge(const Edge<Weight> &edge);

  size_t GetVertexCount() const;
  size_t GetEdgeCount() const;
  const Edge<Weight> &GetEdge(EdgeId edge_id) const;
  IncidentEdges GetIncidentEdges(VertexId vertex) const;

 private:
  std::vector<Edge<Weight>> edges_;
  std::vector<Incidence> incidences_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidences_(vertex_count) {}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight> &edge) {
  edges_.push_back(edge);
  const EdgeId id = edges_.size() - 1;
  incidences_.at(edge.from).push_back(id);
  return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
  return incidences_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
  return edges_.size();
}

template <typename Weight>
const Edge<Weight> &DirectedWeightedGraph<Weight>::GetEdge(
    EdgeId edge_id) const {
  return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdges
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
  return ranges::AsRange(incidences_.at(vertex));
}
}  // namespace graph