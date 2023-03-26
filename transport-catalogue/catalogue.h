#pragma once

#include <deque>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

using Distances = std::vector<std::pair<std::string_view, int>>;

namespace catalogue {

    enum class RouteType { 
        CIRCLE, 
        TWO_DIRECTIONAL 
    };

    struct Bus {
        std::string number;
        RouteType type;
        std::vector<std::string_view> stops;
        std::set<std::string_view> unique_stops;

        [[nodiscard]] size_t GetStopsCount() const;
    };

    struct Stop {
        std::string name;
        catalogue::geo::Coordinates point;

        [[nodiscard]] size_t Hash() const {
            return std::hash<std::string>{}(name) + even_value * std::hash<double>{}(point.lng) + even_value * even_value * std::hash<double>{}(point.lat);
        }

        private:
            static const size_t even_value{37};
    };

    using StopPairs = std::pair<const Stop*, const Stop*>;

    struct BusStat {
        std::string_view number;
        size_t stops_count{0u};
        size_t unique_stops_count{0u};
        int route_length{0};
        double curvature{0.};
    };

    std::ostream& operator<<(std::ostream& os, const BusStat& statistics);

    class TransportCatalogue {
        public:
            TransportCatalogue() = default;
            
            void AddStop(Stop stop);

            void AddBus(Bus bus);
            
            void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);

            [[nodiscard]] std::optional<BusStat> GetBusStat(std::string_view bus_number) const;
            
            [[nodiscard]] const std::set<std::string_view>* GetBusPassStop(std::string_view stop_name) const;

        private:
            struct StopPairsHash {
                size_t operator()(const StopPairs& pair) const {
                    return pair.first->Hash() + even_value * pair.second->Hash();
                }

                private:
                    static const size_t even_value{37};
            };

            int CalculateLength(const Bus* bus_info) const;

            double CalculateGeoLength(const Bus* bus_info) const;

            std::deque<Stop> stops_db_;
            std::deque<Bus> buses_db_;
            std::unordered_map<std::string_view, const Stop*> stops_;
            std::unordered_map<std::string_view, const Bus*> buses_;
            std::unordered_map<std::string_view, std::set<std::string_view>> buses_stops_;
            std::unordered_map<StopPairs, int, StopPairsHash> distances_stops_;
        };

}  // namespace catalogue