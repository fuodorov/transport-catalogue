#pragma once

#include <iostream>
#include <string>

#include "catalogue.h"

namespace catalogue::parser {
    Distances ParseDistances(std::string_view text);
    std::pair<catalogue::Stop, bool> ParseCoordinates(const std::string& text);
    catalogue::Bus ParseRoutes(std::string_view text);
    void ParseQueries(catalogue::TransportCatalogue& catalogue, std::istream& input_stream, std::ostream& output_stream);
}  // namespace catalogue::parser