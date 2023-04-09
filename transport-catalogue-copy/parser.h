#pragma once

#include "json.h"
#include "renderer.h"
#include "catalogue.h"

namespace parser {

catalogue::TransportCatalogue ParseQueries(const json::Array& requests);

renderer::Visualization ParseVisualizationSettings(const json::Dict& settings);

json::Node MakeStatResponse(const catalogue::TransportCatalogue& catalogue, const json::Array& requests,
                            const renderer::Visualization& settings);

}  // namespace parser