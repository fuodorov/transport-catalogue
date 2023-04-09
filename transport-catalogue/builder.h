#pragma once

#include "json.h"
#include "renderer.h"
#include "catalogue.h"

namespace builder {
    json::Node BuildStatResponse(const catalogue::TransportCatalogue& catalogue, const json::Array& requests, const renderer::Visualization& settings);
}  // namespace builder