#pragma once

#include "catalogue.h"
#include "json/builder.h"
#include "renderer.h"
#include "router.h"

using namespace transport_catalogue;
using namespace map_renderer;
using namespace transport_catalogue::json;
using namespace transport_catalogue::json::builder;
using namespace transport_catalogue::router;

namespace handler {

class Handler {
 public:
  Handler() = default;

  std::optional<RouteInfo> GetRouteInfo(std::string_view start,
                                        std::string_view end,
                                        TransportCatalogue &catalogue,
                                        TransportRouter &routing) const;

  std::vector<geo::Coordinates> GetStopsCoordinates(
      TransportCatalogue &catalogue_) const;
  std::vector<std::string_view> GetBusNames(
      TransportCatalogue &catalogue_) const;

  BusInfo BusQuery(TransportCatalogue &catalogue, std::string_view str);
  StopInfo StopQuery(TransportCatalogue &catalogue, std::string_view stop_name);

  Node MakeStopNode(int id_request, const StopInfo &query_result);
  Node MakeBusNode(int id_request, const BusInfo &query_result);
  Node MakeMapNode(int id_request, TransportCatalogue &catalogue,
                   RenderSettings render_settings);
  Node MakeRouteNode(StatisticRequest &request, TransportCatalogue &catalogue,
                     TransportRouter &routing);

  void Queries(TransportCatalogue &catalogue,
               std::vector<StatisticRequest> &stat_requests,
               RenderSettings &render_settings,
               RoutingSettings &route_settings);

  void RenderMap(MapRenderer &map_catalogue,
                 TransportCatalogue &catalogue_) const;

  const Document &GetDocument();

 private:
  Document document;
};

}  // end namespace handler