#pragma once

#include "catalogue.h"
#include "graph/router.h"
#include "domain.h"

#include <deque>
#include <unordered_map>
#include <iostream>

namespace transport_catalogue {
namespace detail {
namespace router {

using namespace domain;
using namespace graph;
    
static const uint16_t KILOMETER = 1000;
static const uint16_t HOUR = 60;    

class TransportRouter {
public:
	void set_routing_settings(RoutingSettings routing_settings);
	const RoutingSettings& get_routing_settings() const;

	void build_router(TransportCatalogue& transport_catalogue);

	const DirectedWeightedGraph<double>& get_graph() const;
    const Router<double>& get_router() const;
    const std::variant<StopEdge, BusEdge>& get_edge(EdgeId id) const;
    
	std::optional<RouterByStop> get_router_by_stop(Stop* stop) const;
	std::optional<RouteInfo> get_route_info(VertexId start, VertexId end) const;

	const std::unordered_map<Stop*, RouterByStop>& get_stop_to_vertex() const;
	const std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>>& get_edge_id_to_edge() const;
    
    std::deque<Stop*> get_stops_ptr(TransportCatalogue& transport_catalogue);
    std::deque<Bus*> get_bus_ptr(TransportCatalogue& transport_catalogue);
        
	void add_edge_to_stop();
	void add_edge_to_bus(TransportCatalogue& transport_catalogue);
    
    void set_stops(const std::deque<Stop*>& stops);
    void set_graph(TransportCatalogue& transport_catalogue);

	Edge<double> make_edge_to_bus(Stop* start, Stop* end, const double distance) const;

	template <typename Iterator>
	void parse_bus_to_edges(Iterator first, 
                            Iterator last,
                            const TransportCatalogue& transport_catalogue, 
                            const Bus* bus);
    
private:    
    std::unordered_map<Stop*, RouterByStop> stop_to_router_;
	std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> edge_id_to_edge_;
    
	std::unique_ptr<DirectedWeightedGraph<double>> graph_;
	std::unique_ptr<Router<double>> router_;
    
    RoutingSettings routing_settings_;
};

template <typename Iterator>
void TransportRouter::parse_bus_to_edges(Iterator first, 
                                         Iterator last,
                                         const TransportCatalogue& transport_catalogue, 
                                         const Bus* bus) {
    
    for (auto it = first; it != last; ++it) {
        size_t distance = 0;
        size_t span = 0;

        for (auto it2 = std::next(it); it2 != last; ++it2) {
            distance += transport_catalogue.get_distance_stop(*prev(it2), *it2);
            ++span;

            EdgeId id = graph_->add_edge(make_edge_to_bus(*it, *it2, distance));
            
            edge_id_to_edge_[id] = BusEdge{bus->name, span, graph_->get_edge(id).weight};
        }
    }
}

}//end namespace router
}//end namespace detail
}//end namespace transport_catalogue