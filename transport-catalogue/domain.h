#pragma once

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"

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
  int rout_length{0};
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
using StringViewPairStorage =
    std::unordered_map<StringViewPair, Type, StringViewPairHash>;

} // namespace catalogue