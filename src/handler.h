#pragma once

#include "catalogue.h"
#include "json/builder.h"
#include "renderer.h"
#include "router.h"

using namespace transport_catalogue;
using namespace map_renderer;
using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail::json::builder;
using namespace transport_catalogue::detail::router;

namespace request_handler {

class RequestHandler {
public:
    RequestHandler() = default;

    std::optional<RouteInfo> get_route_info(std::string_view start, std::string_view end, TransportCatalogue &catalogue,
                                            TransportRouter &routing) const;

    std::vector<geo::Coordinates> get_stops_coordinates(TransportCatalogue &catalogue_) const;
    std::vector<std::string_view> get_sort_buses_names(TransportCatalogue &catalogue_) const;

    BusQueryResult bus_query(TransportCatalogue &catalogue, std::string_view str);
    StopQueryResult stop_query(TransportCatalogue &catalogue, std::string_view stop_name);

    Node execute_make_node_stop(int id_request, const StopQueryResult &query_result);
    Node execute_make_node_bus(int id_request, const BusQueryResult &query_result);
    Node execute_make_node_map(int id_request, TransportCatalogue &catalogue, RenderSettings render_settings);
    Node execute_make_node_route(StatRequest &request, TransportCatalogue &catalogue, TransportRouter &routing);

    void execute_queries(TransportCatalogue &catalogue, std::vector<StatRequest> &stat_requests,
                         RenderSettings &render_settings, RoutingSettings &route_settings);

    void execute_render_map(MapRenderer &map_catalogue, TransportCatalogue &catalogue_) const;

    const Document &get_document();

private:
    Document doc_out;
};

}  // end namespace request_handler