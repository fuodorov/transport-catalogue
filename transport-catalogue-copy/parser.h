#pragma once

#include "json.h"
#include "renderer.h"
#include "catalogue.h"

namespace parser {
    catalogue::TransportCatalogue ParseQueries(const json::Array& requests);
    renderer::Visualization ParseRenderSettings(const json::Dict& settings);
}  // namespace parser