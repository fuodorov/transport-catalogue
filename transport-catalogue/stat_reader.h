#pragma once
#include <string>

#include "transport_catalogue.h"

namespace catalogue::output {
    std::string_view ParseBusStatistics(std::string_view text);
    std::string_view ParseBusPassStop(std::string_view text);
    void PrintBusPassStop(std::ostream& os, std::string_view stop_name, const std::set<std::string_view>* buses);
}  // namespace catalogue::output