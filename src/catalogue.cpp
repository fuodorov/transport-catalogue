#include "catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(Stop &&stop) {
  stops.push_back(std::move(stop));
  Stop *stop_buf = &stops.back();
  stop_names_to_stop.insert(
      transport_catalogue::StopsMap::value_type(stop_buf->name, stop_buf));
}

void TransportCatalogue::AddBus(Bus &&bus) {
  Bus *bus_buf;

  buses.push_back(std::move(bus));
  bus_buf = &buses.back();
  bus_names_to_bus.insert(BusesMap::value_type(bus_buf->name, bus_buf));

  for (Stop *stop : bus_buf->stops) {
    stop->buses.push_back(bus_buf);
  }

  bus_buf->route_length = GetDistanceBuses(bus_buf);
}

void TransportCatalogue::AddDistance(const std::vector<Distance> &distances) {
  for (auto distance : distances) {
    auto dist_pair = std::make_pair(distance.start, distance.end);
    distances_to_stop.insert(
        DistancesMap::value_type(dist_pair, distance.distance));
  }
}

Bus *TransportCatalogue::GetBus(std::string_view bus_name) {
  if (bus_names_to_bus.empty()) {
    return nullptr;
  }

  if (bus_names_to_bus.count(bus_name)) {
    return bus_names_to_bus.at(bus_name);
  } else {
    return nullptr;
  }
}

Stop *TransportCatalogue::GetStop(std::string_view stop_name) {
  if (stop_names_to_stop.empty()) {
    return nullptr;
  }

  if (stop_names_to_stop.count(stop_name)) {
    return stop_names_to_stop.at(stop_name);
  } else {
    return nullptr;
  }
}

std::deque<Stop> TransportCatalogue::GetStops() const { return stops; }

std::deque<Bus> TransportCatalogue::GetBuses() const { return buses; }

BusesMap TransportCatalogue::GetBusNames() const { return bus_names_to_bus; }

StopsMap TransportCatalogue::GetStopNames() const { return stop_names_to_stop; }

std::unordered_set<const Stop *> TransportCatalogue::GetUniqueStops(Bus *bus) {
  std::unordered_set<const Stop *> unique_stops;
  unique_stops.insert(bus->stops.begin(), bus->stops.end());

  return unique_stops;
}

double TransportCatalogue::GetLength(Bus *bus) {
  return transform_reduce(
      next(bus->stops.begin()), bus->stops.end(), bus->stops.begin(), 0.0,
      std::plus<>{}, [](const Stop *lhs, const Stop *rhs) {
        return geo::CalculateDistance({(*lhs).latitude, (*lhs).longitude},
                                      {(*rhs).latitude, (*rhs).longitude});
      });
}

std::unordered_set<const Bus *> TransportCatalogue::GetUniqueBuses(Stop *stop) {
  std::unordered_set<const Bus *> unique_stops;
  unique_stops.insert(stop->buses.begin(), stop->buses.end());

  return unique_stops;
}

DistancesMap TransportCatalogue::GetDistance() const {
  return distances_to_stop;
}

size_t TransportCatalogue::GetDistanceStops(const Stop *begin,
                                            const Stop *finish) const {
  if (distances_to_stop.empty()) {
    return 0;

  } else {
    if (const auto &stop_ptr_pair = std::make_pair(begin, finish);
        distances_to_stop.count(stop_ptr_pair)) {
      return distances_to_stop.at(stop_ptr_pair);

    } else if (const auto &stop_ptr_pair = std::make_pair(finish, begin);
               distances_to_stop.count(stop_ptr_pair)) {
      return distances_to_stop.at(stop_ptr_pair);

    } else {
      return 0;
    }
  }
}

size_t TransportCatalogue::GetDistanceBuses(Bus *bus) {
  size_t distance = 0;
  auto stops_size = bus->stops.size() - 1;

  for (int i = 0; i < static_cast<int>(stops_size); i++) {
    distance += GetDistanceStops(bus->stops[i], bus->stops[i + 1]);
  }

  return distance;
}

}  // end namespace transport_catalogue