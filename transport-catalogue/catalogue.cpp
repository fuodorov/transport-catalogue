#include "catalogue.h"

#include <execution>
#include <iomanip>
#include <numeric>

namespace catalogue {

    std::ostream& operator<<(std::ostream& os, const BusStat& bus) {
        return os << "Bus " << bus.number << ": " << bus.stops_count << " stops on route, "
                  << bus.unique_stops_count << " unique stops, " << bus.route_length << " route length, "
                  << std::setprecision(6) << bus.curvature << " curvature";
    }

    size_t Bus::GetStopsCount() const {
        return (type == RouteType::CIRCLE) ? stops.size() : 2 * stops.size() - 1;
    }

    void TransportCatalogue::AddStop(Stop stop) {
        const auto position = stops_db_.insert(stops_db_.begin(), std::move(stop));
        stops_.insert({position->name, &(*position)});
        buses_stops_.insert({position->name, {}});
    }

    void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
        distances_stops_.insert({{stops_.at(stop_from), stops_.at(stop_to)}, distance});
    }

    void TransportCatalogue::AddBus(Bus bus) {
        for (auto& stop : bus.stops)
            stop = stops_.find(stop)->first;
        bus.unique_stops = {bus.stops.begin(), bus.stops.end()};

        const auto position = buses_db_.insert(buses_db_.begin(), std::move(bus));
        buses_.insert({position->number, &(*position)});

        for (std::string_view stop : position->stops)
            buses_stops_[stop].insert(position->number);
    }

    std::optional<BusStat> TransportCatalogue::GetBusStat(std::string_view bus_number) const {
        if (buses_.count(bus_number) == 0)
            return std::nullopt;

        const Bus* bus = buses_.at(bus_number);

        return BusStat{bus->number, bus->GetStopsCount(), bus->unique_stops.size(), CalculateLength(bus), static_cast<double>(CalculateLength(bus)) / CalculateGeoLength(bus)};
    }

    int TransportCatalogue::CalculateLength(const Bus* bus) const {
        auto get_route_len = [this](std::string_view from, std::string_view to) {
            auto key = std::make_pair(stops_.at(from), stops_.at(to));
            return (distances_stops_.count(key) > 0) ? distances_stops_.at(key) : distances_stops_.at({stops_.at(to), stops_.at(from)});
        };

        int forward_route = std::transform_reduce(bus->stops.begin(), std::prev(bus->stops.end()), std::next(bus->stops.begin()), 0, std::plus<>(), get_route_len);
        int backward_route = std::transform_reduce(bus->stops.rbegin(), std::prev(bus->stops.rend()), std::next(bus->stops.rbegin()), 0, std::plus<>(), get_route_len);

        return (bus->type == RouteType::CIRCLE) ? forward_route : forward_route + backward_route;
    }

    double TransportCatalogue::CalculateGeoLength(const Bus* bus) const {
        auto get_geographic_len = [this](std::string_view from, std::string_view to) {
            return geo::ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
        };

        double geographic_length = std::transform_reduce(bus->stops.begin(), std::prev(bus->stops.end()), std::next(bus->stops.begin()), 0., std::plus<>(), get_geographic_len);

        return (bus->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.;
    }

    const std::set<std::string_view>* TransportCatalogue::GetBusPassStop(std::string_view stop_name) const {
        return (buses_stops_.count(stop_name) > 0) ? &buses_stops_.at(stop_name) : nullptr;
    }

}  // namespace catalogue