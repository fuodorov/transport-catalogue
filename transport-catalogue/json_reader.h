#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace request {

struct ResponseSettings {
  routing::Settings routing;
  render::Visualization visualization;
};

catalogue::TransportCatalogue ParseBaseRequest(const json::Array &requests);

render::Visualization ParseRenderSettings(const json::Dict &settings);

routing::Settings ParseRoutingSettings(const json::Dict &requests);

json::Node MakeResponse(const catalogue::TransportCatalogue &catalogue,
                        routing::TransportRouterOpt &router,
                        const json::Array &requests,
                        const ResponseSettings &settings);

void RequestHandler(std::istream &input, std::ostream &output);

} // namespace request