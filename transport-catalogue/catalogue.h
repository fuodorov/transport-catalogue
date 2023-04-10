#pragma once

#include <deque>
#include <map>
#include <optional>
#include <unordered_map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
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
            return std::hash<std::string>{}(name) + even_value * std::hash<double>{}(point.lng) + even_value * even_value * std::hash<double>{}(point.lat);
        }

    private:
        static const size_t even_value{37};
    };

    using StopPairs = std::pair<std::shared_ptr<Stop>, std::shared_ptr<Stop>>;

    struct BusStat {
        std::string_view number;
        size_t stops_count{0u}, unique_stops_count{0u};
        int route_length{0};
        double curvature{0.};
    };

    std::ostream& operator<<(std::ostream& os, const BusStat& statistics);

    using BusStopsStorage = std::pair<std::shared_ptr<Bus>, std::vector<std::shared_ptr<Stop>>>;
    using StopsStorage = std::map<std::string_view, std::shared_ptr<Stop>>;

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;

        void AddStop(Stop stop);
        void AddBus(Bus bus);
        void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);

        [[nodiscard]] std::optional<BusStat> GetBusStat(std::string_view bus_number) const;
        [[nodiscard]] std::unique_ptr<std::set<std::string_view>> GetBusPassStop(std::string_view stop_name) const;
        [[nodiscard]] const geo::Coordinates& GetMinStopCoordinates() const;
        [[nodiscard]] const geo::Coordinates& GetMaxStopCoordinates() const;
        [[nodiscard]] const std::set<std::string_view>& GetOrderedBusList() const;
        [[nodiscard]] BusStopsStorage GetFinalStops(std::string_view bus_name) const;
        [[nodiscard]] BusStopsStorage GetRouteInfo(std::string_view bus_name, bool include_backward_way = true) const;
        [[nodiscard]] StopsStorage GetAllStopsFromRoutes() const;

    private:
        struct StopPairsHash {
            size_t operator()(const StopPairs& pair) const {
                return pair.first->Hash() + prime_number * pair.second->Hash();
            }

            private:
                static const size_t prime_number{31};
        };

        [[nodiscard]] int CalculateLength(const std::shared_ptr<Bus>& bus_info) const;
        [[nodiscard]] double CalculateGeoLength(const std::shared_ptr<Bus>& bus_info) const;

        void UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates);

        std::deque<Stop> stops_db_;
        std::deque<Bus> buses_db_;
        std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops_;
        std::unordered_map<std::string_view, std::shared_ptr<Bus>> buses_;
        std::unordered_map<std::string_view, std::set<std::string_view>> buses_stops_;
        std::unordered_map<StopPairs, int, StopPairsHash> distances_stops_;
        geo::Coordinates coordinates_min_{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
        geo::Coordinates coordinates_max_{std::numeric_limits<double>::min(), std::numeric_limits<double>::min()};
        std::set<std::string_view> ordered_bus_list_;
    };

}  // namespace catalogue