#pragma once

#include "catalogue.h"
#include "json/builder.h"
#include "renderer.h"
#include "router.h"

using namespace transport_catalogue;
using namespace renderer;
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

  BusInfo BusQuery(TransportCatalogue &catalogue, std::string_view name);
  StopInfo StopQuery(TransportCatalogue &catalogue, std::string_view name);

  Node MakeStopNode(int request_id, const StopInfo &query);
  Node MakeBusNode(int request_id, const BusInfo &query);
  Node MakeMapNode(int request_id, TransportCatalogue &catalogue,
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