#pragma once

#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo/geo.h"

namespace catalogue {

enum class RouteType { CIRCLE, TWO_DIRECTIONAL };

struct Bus {
  std::string number;
  RouteType type;
  std::vector<std::string_view> stops;
  std::set<std::string_view> unique_stops;

  [[nodiscard]] size_t GetStopsCount() const;
};

struct Stop {
  std::string name;
  geo::Coordinates point;

  [[nodiscard]] size_t Hash() const {
    return std::hash<std::string>{}(name) +
           even_ * std::hash<double>{}(point.lng) +
           even_ * even_ * std::hash<double>{}(point.lat);
  }

private:
  static const size_t even_{42};
};

struct BusStatistics {
  std::string_view number;
  size_t stops_count{0u};
  size_t unique_stops_count{0u};
  int rout_len{0};
  double curvature{0.};
};

std::ostream &operator<<(std::ostream &os, const BusStatistics &statistics);

using StringViewPair = std::pair<std::string_view, std::string_view>;

struct StringViewPairHash {
  size_t operator()(const StringViewPair &pair) const {
    return even_ * std::hash<std::string_view>{}(pair.first) +
           even_ * even_ * std::hash<std::string_view>{}(pair.second);
  }

private:
  static constexpr size_t even_{42};
};

template <class Type>
using StringViewPairDB =
    std::unordered_map<StringViewPair, Type, StringViewPairHash>;
using BusStops =
    std::pair<std::shared_ptr<Bus>, std::vector<std::shared_ptr<Stop>>>;
using Stops = std::map<std::string_view, std::shared_ptr<Stop>>;

struct TempInfo {
  double time{0.};
  int stops_count{0};
};

class TransportCatalogue {
public:
  TransportCatalogue() = default;

  void AddStop(Stop stop);
  void AddBus(Bus bus);
  void AddDistance(std::string_view stop_from, std::string_view stop_to,
                   int distance);

  [[nodiscard]] std::optional<BusStatistics>
  GetBusStat(std::string_view bus_number) const;
  [[nodiscard]] std::unique_ptr<std::set<std::string_view>>
  GetBusPassStop(std::string_view stop_name) const;

  [[nodiscard]] const geo::Coordinates &GetMinCoordinates() const;
  [[nodiscard]] const geo::Coordinates &GetMaxCoordinates() const;

  [[nodiscard]] const std::set<std::string_view> &GetOrderedBuses() const;
  [[nodiscard]] BusStops GetFinalStops(std::string_view bus_name) const;
  [[nodiscard]] BusStops
  GetRouteTempInfo(std::string_view bus_name,
                   bool include_backward_way = true) const;
  [[nodiscard]] Stops GetAllStops() const;

  [[nodiscard]] std::set<std::string_view> GetUniqueStops() const;
  [[nodiscard]] const std::deque<Bus> &GetBuses() const;
  [[nodiscard]] StringViewPairDB<TempInfo>
  GetAllDistances(std::string_view bus_number, double bus_velocity) const;

private:
  [[nodiscard]] int
  CalculateRouteLen(const std::shared_ptr<Bus> &bus_info) const;
  [[nodiscard]] double
  CalculateGeoLen(const std::shared_ptr<Bus> &bus_info) const;

  void UpdateMinMaxCoordinates(const geo::Coordinates &coordinates);

  using StopPointersPair =
      std::pair<std::shared_ptr<Stop>, std::shared_ptr<Stop>>;

  struct StopPointersPairHash {
    size_t operator()(const StopPointersPair &pair) const {
      return pair.first->Hash() + even_ * pair.second->Hash();
    }

  private:
    static const size_t even_{42};
  };

  template <class Type>
  using InterStops =
      std::unordered_map<StopPointersPair, Type, StopPointersPairHash>;

  std::deque<Stop> stops_db_;
  std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops_;

  std::deque<Bus> buses_db_;
  std::unordered_map<std::string_view, std::shared_ptr<Bus>> buses_;

  std::unordered_map<std::string_view, std::set<std::string_view>>
      buses_through_stop_;
  InterStops<int> distances_stops_;

  geo::Coordinates min_coordinates_{std::numeric_limits<double>::max(),
                                    std::numeric_limits<double>::max()};
  geo::Coordinates max_coordinates_{std::numeric_limits<double>::min(),
                                    std::numeric_limits<double>::min()};

  std::set<std::string_view> ordered_buses_;
};

} // namespace catalogue