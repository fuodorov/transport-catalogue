#include "catalogue.h"

#include <execution>
#include <numeric>

namespace catalogue {

void TransportCatalogue::AddStop(Stop stop) {
  const auto position = stops_db_.insert(stops_db_.begin(), std::move(stop));
  stops_.insert({position->name, std::make_shared<Stop>(*position)});
  buses_through_stop_.insert({position->name, {}});
}

void TransportCatalogue::AddDistance(std::string_view stop_from,
                                     std::string_view stop_to, int distance) {
  distances_stops_.insert(
      {{stops_.at(stop_from), stops_.at(stop_to)}, distance});
}

void TransportCatalogue::AddBus(Bus bus) {
  for (auto &stop : bus.stops) {
    UpdateMinMaxCoordinates(stops_.at(stops_.find(stop)->first)->point);
  }
  bus.unique_stops = {bus.stops.begin(), bus.stops.end()};

  const auto position = buses_db_.insert(buses_db_.begin(), std::move(bus));
  buses_.insert({position->number, std::make_shared<Bus>(*position)});
  ordered_buses_.emplace(position->number);

  for (std::string_view stop : position->stops) {
    buses_through_stop_[stop].insert(position->number);
  }
}

std::optional<BusStatistics>
TransportCatalogue::GetBusStat(std::string_view bus_number) const {
  if (buses_.count(bus_number) == 0) {
    return std::nullopt;
  }

  auto bus_info = buses_.at(bus_number);
  return BusStatistics{bus_info->number, bus_info->GetStopsCount(),
                       bus_info->unique_stops.size(),
                       CalculateRouteLen(bus_info), CalculateGeoLen(bus_info)};
}

int TransportCatalogue::CalculateRouteLen(
    const std::shared_ptr<Bus> &bus_info) const {
  auto get_route_len = [this](std::string_view from, std::string_view to) {
    auto key = std::make_pair(stops_.at(from), stops_.at(to));
    return (distances_stops_.count(key) > 0)
               ? distances_stops_.at(key)
               : distances_stops_.at({stops_.at(to), stops_.at(from)});
  };

  int forward_route = std::transform_reduce(
      bus_info->stops.begin(), std::prev(bus_info->stops.end()),
      std::next(bus_info->stops.begin()), 0, std::plus<>(), get_route_len);

  int backward_route = std::transform_reduce(
      bus_info->stops.rbegin(), std::prev(bus_info->stops.rend()),
      std::next(bus_info->stops.rbegin()), 0, std::plus<>(), get_route_len);

  return (bus_info->type == RouteType::CIRCLE) ? forward_route
                                               : forward_route + backward_route;
}

double TransportCatalogue::CalculateGeoLen(
    const std::shared_ptr<Bus> &bus_info) const {
  double geo_len = std::transform_reduce(
      std::next(bus_info->stops.begin()), bus_info->stops.end(),
      bus_info->stops.begin(), 0., std::plus<>(),
      [this](std::string_view from, std::string_view to) {
        return CalculateDistance(stops_.at(from)->point, stops_.at(to)->point);
      });

  return (bus_info->type == RouteType::CIRCLE) ? geo_len : geo_len * 2.;
}

void TransportCatalogue::UpdateMinMaxCoordinates(
    const geo::Coordinates &coordinates) {
  min_coordinates_.lat = std::min(min_coordinates_.lat, coordinates.lat);
  min_coordinates_.lng = std::min(min_coordinates_.lng, coordinates.lng);

  max_coordinates_.lat = std::max(max_coordinates_.lat, coordinates.lat);
  max_coordinates_.lng = std::max(max_coordinates_.lng, coordinates.lng);
}

const geo::Coordinates &TransportCatalogue::GetMinCoordinates() const {
  return min_coordinates_;
}

const geo::Coordinates &TransportCatalogue::GetMaxCoordinates() const {
  return max_coordinates_;
}

const std::set<std::string_view> &TransportCatalogue::GetOrderedBuses() const {
  return ordered_buses_;
}

BusStops TransportCatalogue::GetFinalStops(std::string_view bus_name) const {
  auto bus = buses_.at(bus_name);

  std::vector<std::shared_ptr<Stop>> stops;

  if (bus->stops.empty())
    return std::make_pair(bus, stops);

  if (bus->type == RouteType::CIRCLE) {
    stops.emplace_back(stops_.at(bus->stops.front()));
  } else if (bus->type == RouteType::TWO_DIRECTIONAL) {
    stops.emplace_back(stops_.at(bus->stops.front()));

    if (bus->stops.front() != bus->stops.back())
      stops.emplace_back(stops_.at(bus->stops.back()));
  }

  return std::make_pair(bus, stops);
}

BusStops TransportCatalogue::GetRouteTempInfo(std::string_view bus_name,
                                              bool include_backward_way) const {
  auto bus = buses_.at(bus_name);

  std::vector<std::shared_ptr<Stop>> stops;
  stops.reserve(bus->GetStopsCount());

  for (std::string_view stop : bus->stops)
    stops.emplace_back(stops_.at(stop));

  if (include_backward_way &&
      bus->type == catalogue::RouteType::TWO_DIRECTIONAL) {
    for (auto stop = std::next(bus->stops.rbegin()); stop != bus->stops.rend();
         ++stop)
      stops.emplace_back(stops_.at(*stop));
  }

  return std::make_pair(std::move(bus), std::move(stops));
}

Stops TransportCatalogue::GetAllStops() const {
  std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops;

  for (const auto &[_, bus] : buses_) {
    for (std::string_view stop : bus->stops) {
      if (stops.count(stop) == 0)
        stops.emplace(stop, stops_.at(stop));
    }
  }

  return {stops.begin(), stops.end()};
}

std::set<std::string_view> TransportCatalogue::GetUniqueStops() const {
  std::set<std::string_view> stops;

  for (auto [stop_name, _] : stops_)
    stops.emplace(stop_name);

  return stops;
}

const std::deque<Bus> &TransportCatalogue::GetBuses() const {
  return buses_db_;
}

StringViewPairDB<TempInfo>
TransportCatalogue::GetAllDistances(std::string_view bus_number,
                                    double bus_velocity) const {
  StringViewPairDB<TempInfo> distances;

  auto get_time = [this, &bus_velocity](std::string_view from,
                                        std::string_view to) -> double {
    auto key = std::make_pair(stops_.at(from), stops_.at(to));
    return (distances_stops_.count(key) > 0)
               ? distances_stops_.at(key) / bus_velocity
               : distances_stops_.at({stops_.at(to), stops_.at(from)}) /
                     bus_velocity;
  };

  auto add_info = [&distances, &get_time](auto begin, auto end) {
    double cumulative_time{0.};
    StringViewPair key;
    TempInfo current_info;
    auto previous{begin};

    for (auto from = begin; from != end; ++from) {
      cumulative_time = 0.;
      previous = from;

      for (auto to = std::next(from); to != end; ++to) {
        cumulative_time += get_time(*previous, *to);
        key = StringViewPair{*from, *to};
        current_info = TempInfo{cumulative_time,
                                static_cast<int>(std::distance(from, to))};

        if (distances.count(key) > 0) {
          distances[key] = distances[key].time < cumulative_time
                               ? distances[key]
                               : current_info;
        } else {
          distances.emplace(key, current_info);
        }

        previous = to;
      }
    }
  };

  const auto &bus = buses_.at(bus_number);
  const auto &stops = bus->stops;

  if (bus->type == RouteType::TWO_DIRECTIONAL) {
    add_info(stops.begin(), stops.end());
    add_info(stops.rbegin(), stops.rend());

  } else if (bus->type == RouteType::CIRCLE) {
    add_info(stops.begin(), stops.end());
  }

  return distances;
}

std::unique_ptr<std::set<std::string_view>>
TransportCatalogue::GetBusPassStop(std::string_view stop_name) const {
  if (const auto position = buses_through_stop_.find(stop_name);
      position != buses_through_stop_.end())
    return std::make_unique<std::set<std::string_view>>(position->second);
  return nullptr;
}

} // namespace catalogue