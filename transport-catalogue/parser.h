#pragma once

#include <iostream>
#include <string>

#include "catalogue.h"

namespace catalogue::parser {
    std::string_view RemoveFirstWord(std::string_view text);
    DistancesToStops ParseDistances(std::string_view text);
    std::pair<catalogue::Stop, bool> ParseBusStop(const std::string& text);
    catalogue::Bus ParseBusRoute(std::string_view text);
    void ParseTransportCatalogueQueries(catalogue::TransportCatalogue& catalogue, std::istream& input_stream, std::ostream& output_stream);
}  // namespace catalogue::parser