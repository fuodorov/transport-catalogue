#pragma once
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"

using namespace domain;

namespace transport_catalogue {

struct DistancesHasher {
  std::hash<const void *> hasher;

  std::size_t operator()(
      const std::pair<const Stop *, const Stop *> stops_pair) const noexcept {
    return hasher(static_cast<const void *>(stops_pair.first)) * 42 +
           hasher(static_cast<const void *>(stops_pair.second));
  }
};

typedef std::unordered_map<std::string_view, Stop *> StopsMap;
typedef std::unordered_map<std::string_view, Bus *> BusesMap;
typedef std::unordered_map<std::pair<const Stop *, const Stop *>, int,
                           DistancesHasher>
    DistancesMap;

class TransportCatalogue {
 public:
  void AddBus(Bus &&bus);
  void AddStop(Stop &&stop);
  void AddDistance(const std::vector<Distance> &distances);

  Bus *GetBus(std::string_view bus_name);
  Stop *GetStop(std::string_view stop_name);

  std::deque<Stop> GetStops() const;
  std::deque<Bus> GetBuses() const;

  BusesMap GetBusNames() const;
  StopsMap GetStopNames() const;

  std::unordered_set<const Bus *> GetUniqueBuses(Stop *stop);
  std::unordered_set<const Stop *> GetUniqueStops(Bus *bus);
  double GetLength(Bus *bus);

  DistancesMap GetDistance() const;
  size_t GetDistanceStops(const Stop *start, const Stop *finish) const;
  size_t GetDistanceBuses(Bus *bus);

 private:
  std::deque<Stop> stops;
  StopsMap stop_names_to_stop;

  std::deque<Bus> buses;
  BusesMap bus_names_to_bus;

  DistancesMap distances_to_stop;
};

}  // end namespace transport_catalogue