#include "router.h"

namespace transport_catalogue {
namespace detail {
namespace router {

void TransportRouter::set_routing_settings(RoutingSettings routing_settings) {routing_settings_ = std::move(routing_settings);}
const RoutingSettings& TransportRouter::get_routing_settings() const {return routing_settings_;}

void TransportRouter::build_router(TransportCatalogue& transport_catalogue) {
	set_graph(transport_catalogue);
	router_ = std::make_unique<Router<double>>(*graph_);
	router_->build();
}

const DirectedWeightedGraph<double>& TransportRouter::get_graph() const {return *graph_;}
const Router<double>& TransportRouter::get_router() const {return *router_;}
const std::variant<StopEdge, BusEdge>& TransportRouter::get_edge(EdgeId id) const {return edge_id_to_edge_.at(id);}

std::optional<RouterByStop> TransportRouter::get_router_by_stop(Stop* stop) const {
	if (stop_to_router_.count(stop)) {
        return stop_to_router_.at(stop);
	} else {
        return std::nullopt;
    }
}

std::optional<RouteInfo> TransportRouter::get_route_info(VertexId start, graph::VertexId end) const {
	const auto& route_info = router_->build_route(start, end);
	if (route_info) {
        RouteInfo result;
        result.total_time = route_info->weight;
        
        for (const auto edge : route_info->edges) {
            result.edges.emplace_back(get_edge(edge));
        }
        
        return result;
        
	} else {
        return std::nullopt;
    }
}   
    
const std::unordered_map<Stop*, RouterByStop>& TransportRouter::get_stop_to_vertex() const {return stop_to_router_;}
const std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>>& TransportRouter::get_edge_id_to_edge() const {return edge_id_to_edge_;}
    
std::deque<Stop*> TransportRouter::get_stops_ptr(TransportCatalogue& transport_catalogue) {
    std::deque<Stop*> stops_ptr;  
    
    for (auto [_, stop_ptr] : transport_catalogue.get_stopname_to_stop()) {
        stops_ptr.push_back(stop_ptr);
    }

    return stops_ptr;
}
    
std::deque<Bus*> TransportRouter::get_bus_ptr(TransportCatalogue& transport_catalogue) {
    std::deque<Bus*> buses_ptr;  
    
    for (auto [_, bus_ptr] : transport_catalogue.get_busname_to_bus()) {
        buses_ptr.push_back(bus_ptr);
    }

    return buses_ptr;
}
    
void TransportRouter::set_stops(const std::deque<Stop*>& stops) {
	size_t i = 0;
    
	for (const auto stop : stops) {
		VertexId first = i++;
		VertexId second = i++;
        
		stop_to_router_[stop] = RouterByStop{first, second};
	}
}

void TransportRouter::add_edge_to_stop() {   
    
	for (const auto [stop, num] : stop_to_router_) {
		EdgeId id = graph_->add_edge(Edge<double>{num.bus_wait_start, 
                                                 num.bus_wait_end, 
                                                 routing_settings_.bus_wait_time});
        
		edge_id_to_edge_[id] = StopEdge{stop->name, routing_settings_.bus_wait_time};
	}
}

void TransportRouter::add_edge_to_bus(TransportCatalogue& transport_catalogue) {

	for (auto bus : get_bus_ptr(transport_catalogue)) {        
		parse_bus_to_edges(bus->stops.begin(), 
                           bus->stops.end(), 
                           transport_catalogue,
                           bus);
        
		if (!bus->is_roundtrip) {
			parse_bus_to_edges(bus->stops.rbegin(), 
                               bus->stops.rend(), 
                               transport_catalogue,
                               bus);
		}
	}
}

void TransportRouter::set_graph(TransportCatalogue& transport_catalogue) {
    const auto stops_ptr_size = get_stops_ptr(transport_catalogue).size();  
    
	graph_ = std::make_unique<DirectedWeightedGraph<double>>(2 * stops_ptr_size);    
    
	set_stops(get_stops_ptr(transport_catalogue));    
	add_edge_to_stop();
	add_edge_to_bus(transport_catalogue);
}

Edge<double> TransportRouter::make_edge_to_bus(Stop* start, Stop* end, const double distance) const {
	Edge<double> result;
    
	result.from = stop_to_router_.at(start).bus_wait_end;
	result.to = stop_to_router_.at(end).bus_wait_start;
	result.weight = distance * 1.0 / (routing_settings_.bus_velocity * KILOMETER / HOUR);
    
	return result;
}

}//end namespace router
}//end namespace detail
}//end namespace transport_catalogue