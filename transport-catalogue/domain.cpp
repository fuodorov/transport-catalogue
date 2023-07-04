#include "domain.h"

#include <iomanip>

namespace catalogue {

std::ostream &operator<<(std::ostream &os, const BusStatistics &bus_info) {
  return os << "Bus " << bus_info.number << ": " << bus_info.stops_count
            << " stops on route, " << bus_info.unique_stops_count
            << " unique stops, " << bus_info.rout_len << " route len, "
            << std::setprecision(6) << bus_info.curvature << " curvature";
}

size_t Bus::GetStopsCount() const {
  return (type == RouteType::CIRCLE) ? stops.size() : 2 * stops.size() - 1;
}

} // namespace catalogue
