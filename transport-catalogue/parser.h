#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"

namespace catalogue::parser {
    std::string_view ParseBusStatistics(std::string_view text);
    std::string_view ParseBusPassStop(std::string_view text);
    DistancesToStops ParseDistances(std::string_view text);
    std::pair<catalogue::Stop, bool> ParseBusStop(const std::string& text);
    catalogue::Bus ParseBusRoute(std::string_view text);
    void ParseTransportCatalogueQueries(std::istream& input_stream);
}  // namespace catalogue::parser