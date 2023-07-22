#include "catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(Stop &&stop) {
  stops.push_back(std::move(stop));
  Stop *tmp_stop = &stops.back();
  stops_to_stop.insert(StopsMap::value_type(tmp_stop->name, tmp_stop));
}

void TransportCatalogue::AddBus(Bus &&bus) {
  buses.push_back(std::move(bus));
  Bus *tmp_bus = &buses.back();
  buses_to_bus.insert(BusesMap::value_type(tmp_bus->name, tmp_bus));

  for (Stop *stop : tmp_bus->stops) {
    stop->buses.push_back(tmp_bus);
  }

  tmp_bus->route_length = GetDistanceBuses(tmp_bus);
}

void TransportCatalogue::AddDistance(const std::vector<Distance> &distances) {
  for (auto tmp_distance : distances) {
    distances_to_stop.insert(DistancesMap::value_type(
        std::make_pair(tmp_distance.start, tmp_distance.end),
        tmp_distance.distance));
  }
}

Bus *TransportCatalogue::GetBus(std::string_view bus_name) {
  return buses_to_bus.empty() || !buses_to_bus.count(bus_name)
             ? nullptr
             : buses_to_bus.at(bus_name);
}

Stop *TransportCatalogue::GetStop(std::string_view stop_name) {
  return stops_to_stop.empty() || !stops_to_stop.count(stop_name)
             ? nullptr
             : stops_to_stop.at(stop_name);
}

std::deque<Stop> TransportCatalogue::GetStops() const { return stops; }
std::deque<Bus> TransportCatalogue::GetBuses() const { return buses; }
BusesMap TransportCatalogue::GetBusNames() const { return buses_to_bus; }
StopsMap TransportCatalogue::GetStopNames() const { return stops_to_stop; }

std::unordered_set<const Stop *> TransportCatalogue::GetUniqueStops(Bus *bus) {
  return std::unordered_set<const Stop *>(bus->stops.begin(), bus->stops.end());
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
  return std::unordered_set<const Bus *>(stop->buses.begin(),
                                         stop->buses.end());
}

DistancesMap TransportCatalogue::GetDistance() const {
  return distances_to_stop;
}

size_t TransportCatalogue::GetDistanceStops(const Stop *begin,
                                            const Stop *finish) const {
  return distances_to_stop.count(std::make_pair(begin, finish))
             ? distances_to_stop.at(std::make_pair(begin, finish))
             : distances_to_stop.count(std::make_pair(finish, begin))
                   ? distances_to_stop.at(std::make_pair(finish, begin))
                   : 0;
}

size_t TransportCatalogue::GetDistanceBuses(Bus *bus) {
  return transform_reduce(next(bus->stops.begin()), bus->stops.end(),
                          bus->stops.begin(), 0, std::plus<>{},
                          [this](const Stop *lhs, const Stop *rhs) {
                            return GetDistanceStops(lhs, rhs);
                          });
}

}  // end namespace transport_catalogue