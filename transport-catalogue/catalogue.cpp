#include "catalogue.h"

#include <execution>
#include <iomanip>
#include <numeric>

namespace catalogue {

    std::ostream& operator<<(std::ostream& os, const BusStat& bus_info) {
        os << "Bus " << bus_info.number << ": " << bus_info.stops_count << " stops on route, "
        << bus_info.unique_stops_count << " unique stops, ";
        os << bus_info.route_length << " route length, ";
        os << std::setprecision(6) << bus_info.curvature << " curvature";
        return os;
    }

    size_t Bus::GetStopsCount() const {
        return (type == RouteType::CIRCLE) ? stops.size() : 2 * stops.size() - 1;
    }

    void TransportCatalogue::AddStop(Stop stop) {
        const auto position = stops_db_.insert(stops_db_.begin(), std::move(stop));
        stops_.insert({position->name, &(*position)});
        // Add stop for <stop-bus> correspondence
        buses_stops_.insert({position->name, {}});
    }

    void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
        //! On this step we suppose that ALL stops have been parsed
        distances_stops_.insert({{stops_.at(stop_from), stops_.at(stop_to)}, distance});
    }

    void TransportCatalogue::AddBus(Bus bus) {
        //! On this step we suppose that ALL stops have been parsed
        for (auto& stop : bus.stops)
            stop = stops_.find(stop)->first;
        bus.unique_stops = {bus.stops.begin(), bus.stops.end()};

        const auto position = buses_db_.insert(buses_db_.begin(), std::move(bus));
        buses_.insert({position->number, &(*position)});

        // Add stop for <stop-bus> correspondence
        for (std::string_view stop : position->stops)
            buses_stops_[stop].insert(position->number);
    }

    std::optional<BusStat> TransportCatalogue::GetBusStat(std::string_view bus_number) const {
        if (buses_.count(bus_number) == 0)
            return std::nullopt;

        const Bus* bus_info = buses_.at(bus_number);

        BusStat result;
        result.number = bus_info->number;
        result.stops_count = bus_info->GetStopsCount();
        result.unique_stops_count = bus_info->unique_stops.size();
        result.route_length = CalculateLength(bus_info);
        result.curvature = static_cast<double>(result.route_length) / CalculateGeoLength(bus_info);

        return result;
    }

    int TransportCatalogue::CalculateLength(const Bus* bus_info) const {
        auto get_route_length = [this](std::string_view from, std::string_view to) {
            auto key = std::make_pair(stops_.at(from), stops_.at(to));
            // If we not found 'from -> to' than we are looking for 'to -> from'
            return (distances_stops_.count(key) > 0)
                    ? distances_stops_.at(key)
                    : distances_stops_.at({stops_.at(to), stops_.at(from)});
        };

        int forward_route =
            std::transform_reduce(bus_info->stops.begin(), std::prev(bus_info->stops.end()),
                                std::next(bus_info->stops.begin()), 0, std::plus<>(), get_route_length);
        if (bus_info->type == RouteType::CIRCLE)
            return forward_route;

        // Otherwise, this is a two-directional way, so we need to calculate the distance on backward way

        int backward_route =
            std::transform_reduce(bus_info->stops.rbegin(), std::prev(bus_info->stops.rend()),
                                std::next(bus_info->stops.rbegin()), 0, std::plus<>(), get_route_length);

        return forward_route + backward_route;
    }

    double TransportCatalogue::CalculateGeoLength(const Bus* bus_info) const {
        double geographic_length = std::transform_reduce(
            std::next(bus_info->stops.begin()), bus_info->stops.end(), bus_info->stops.begin(), 0.,
            std::plus<>(), [this](std::string_view from, std::string_view to) {
                return catalogue::geo::ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
            });

        return (bus_info->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.;
    }

    const std::set<std::string_view>* TransportCatalogue::GetBusPassStop(std::string_view stop_name) const {
        if (const auto position = buses_stops_.find(stop_name); position != buses_stops_.cend())
            return &position->second;
        return nullptr;
    }

}  // namespace catalogue