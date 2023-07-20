#include "handler.h"

namespace request_handler {

struct EdgeInfoGetter {
    Node operator()(const StopEdge &edge_info) {
        using namespace std::literals;

        return Builder{}
            .start_dict()
            .key("type")
            .value("Wait")
            .key("stop_name")
            .value(std::string(edge_info.name))
            .key("time")
            .value(edge_info.time)
            .end_dict()
            .build();
    }

    Node operator()(const BusEdge &edge_info) {
        using namespace std::literals;

        return Builder{}
            .start_dict()
            .key("type")
            .value("Bus")
            .key("bus")
            .value(std::string(edge_info.bus_name))
            .key("span_count")
            .value(static_cast<int>(edge_info.span_count))
            .key("time")
            .value(edge_info.time)
            .end_dict()
            .build();
    }
};

Node RequestHandler::execute_make_node_stop(int id_request, const StopQueryResult &stop_info) {
    Node result;
    Array buses;
    Builder builder;

    std::string str_not_found = "not found";

    if (stop_info.not_found) {
        builder.start_dict().key("request_id").value(id_request).key("error_message").value(str_not_found).end_dict();

        result = builder.build();

    } else {
        builder.start_dict().key("request_id").value(id_request).key("buses").start_array();

        for (std::string bus_name : stop_info.buses_name) {
            builder.value(bus_name);
        }

        builder.end_array().end_dict();

        result = builder.build();
    }

    return result;
}

Node RequestHandler::execute_make_node_bus(int id_request, const BusQueryResult &bus_info) {
    Node result;
    std::string str_not_found = "not found";

    if (bus_info.not_found) {
        result = Builder{}
                     .start_dict()
                     .key("request_id")
                     .value(id_request)
                     .key("error_message")
                     .value(str_not_found)
                     .end_dict()
                     .build();
    } else {
        result = Builder{}
                     .start_dict()
                     .key("request_id")
                     .value(id_request)
                     .key("curvature")
                     .value(bus_info.curvature)
                     .key("route_length")
                     .value(bus_info.route_length)
                     .key("stop_count")
                     .value(bus_info.stops_on_route)
                     .key("unique_stop_count")
                     .value(bus_info.unique_stops)
                     .end_dict()
                     .build();
    }

    return result;
}

Node RequestHandler::execute_make_node_map(int id_request, TransportCatalogue &catalogue_,
                                           RenderSettings render_settings) {
    Node result;

    std::ostringstream map_stream;
    std::string map_str;

    MapRenderer map_catalogue(render_settings);

    map_catalogue.init_sphere_projector(get_stops_coordinates(catalogue_));

    execute_render_map(map_catalogue, catalogue_);
    map_catalogue.get_stream_map(map_stream);
    map_str = map_stream.str();

    result = Builder{}.start_dict().key("request_id").value(id_request).key("map").value(map_str).end_dict().build();

    return result;
}

Node RequestHandler::execute_make_node_route(StatRequest &request, TransportCatalogue &catalogue,
                                             TransportRouter &routing) {
    const auto &route_info = get_route_info(request.from, request.to, catalogue, routing);

    if (!route_info) {
        return Builder{}
            .start_dict()
            .key("request_id")
            .value(request.id)
            .key("error_message")
            .value("not found")
            .end_dict()
            .build();
    }

    Array items;
    for (const auto &item : route_info->edges) {
        items.emplace_back(std::visit(EdgeInfoGetter{}, item));
    }

    return Builder{}
        .start_dict()
        .key("request_id")
        .value(request.id)
        .key("total_time")
        .value(route_info->total_time)
        .key("items")
        .value(items)
        .end_dict()
        .build();
}

void RequestHandler::execute_queries(TransportCatalogue &catalogue, std::vector<StatRequest> &stat_requests,
                                     RenderSettings &render_settings, RoutingSettings &routing_settings) {
    std::vector<Node> result_request;
    TransportRouter transport_router;

    transport_router.set_routing_settings(routing_settings);
    transport_router.build_router(catalogue);

    for (StatRequest req : stat_requests) {
        if (req.type == "Stop") {
            result_request.push_back(execute_make_node_stop(req.id, stop_query(catalogue, req.name)));

        } else if (req.type == "Bus") {
            result_request.push_back(execute_make_node_bus(req.id, bus_query(catalogue, req.name)));

        } else if (req.type == "Map") {
            result_request.push_back(execute_make_node_map(req.id, catalogue, render_settings));

        } else if (req.type == "Route") {
            result_request.push_back(execute_make_node_route(req, catalogue, transport_router));
        }
    }

    doc_out = Document{Node(result_request)};
}

void RequestHandler::execute_render_map(MapRenderer &map_catalogue, TransportCatalogue &catalogue) const {
    std::vector<std::pair<Bus *, int>> buses_palette;
    std::vector<Stop *> stops_sort;
    int palette_size = 0;
    int palette_index = 0;

    palette_size = map_catalogue.get_palette_size();

    if (palette_size == 0) {
        std::cout << "color palette is empty";
        return;
    }

    auto buses = catalogue.get_busname_to_bus();
    if (buses.size() > 0) {
        for (std::string_view bus_name : get_sort_buses_names(catalogue)) {
            Bus *bus_info = catalogue.get_bus(bus_name);

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
            map_catalogue.add_line(buses_palette);
            map_catalogue.add_buses_name(buses_palette);
        }
    }

    auto stops = catalogue.get_stopname_to_stop();
    if (stops.size() > 0) {
        std::vector<std::string_view> stops_name;

        for (auto &[stop_name, stop] : stops) {
            if (stop->buses.size() > 0) {
                stops_name.push_back(stop_name);
            }
        }

        std::sort(stops_name.begin(), stops_name.end());

        for (std::string_view stop_name : stops_name) {
            Stop *stop = catalogue.get_stop(stop_name);
            if (stop) {
                stops_sort.push_back(stop);
            }
        }

        if (stops_sort.size() > 0) {
            map_catalogue.add_stops_circle(stops_sort);
            map_catalogue.add_stops_name(stops_sort);
        }
    }
}

std::optional<RouteInfo> RequestHandler::get_route_info(std::string_view start, std::string_view end,
                                                        TransportCatalogue &catalogue, TransportRouter &routing) const {
    return routing.get_route_info(routing.get_router_by_stop(catalogue.get_stop(start))->bus_wait_start,
                                  routing.get_router_by_stop(catalogue.get_stop(end))->bus_wait_start);
}

std::vector<geo::Coordinates> RequestHandler::get_stops_coordinates(TransportCatalogue &catalogue_) const {
    std::vector<geo::Coordinates> stops_coordinates;
    auto buses = catalogue_.get_busname_to_bus();

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

std::vector<std::string_view> RequestHandler::get_sort_buses_names(TransportCatalogue &catalogue_) const {
    std::vector<std::string_view> buses_names;

    auto buses = catalogue_.get_busname_to_bus();
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

BusQueryResult RequestHandler::bus_query(TransportCatalogue &catalogue, std::string_view bus_name) {
    BusQueryResult bus_info;
    Bus *bus = catalogue.get_bus(bus_name);

    if (bus != nullptr) {
        bus_info.name = bus->name;
        bus_info.not_found = false;
        bus_info.stops_on_route = static_cast<int>(bus->stops.size());
        bus_info.unique_stops = static_cast<int>(catalogue.get_uniq_stops(bus).size());
        bus_info.route_length = static_cast<int>(bus->route_length);
        bus_info.curvature = double(catalogue.get_distance_to_bus(bus) / catalogue.get_length(bus));
    } else {
        bus_info.name = bus_name;
        bus_info.not_found = true;
    }

    return bus_info;
}

StopQueryResult RequestHandler::stop_query(TransportCatalogue &catalogue, std::string_view stop_name) {
    std::unordered_set<const Bus *> unique_buses;
    StopQueryResult stop_info;
    Stop *stop = catalogue.get_stop(stop_name);

    if (stop != NULL) {
        stop_info.name = stop->name;
        stop_info.not_found = false;
        unique_buses = catalogue.stop_get_uniq_buses(stop);

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

const Document &RequestHandler::get_document() {
    return doc_out;
}

}  // end namespace request_handler