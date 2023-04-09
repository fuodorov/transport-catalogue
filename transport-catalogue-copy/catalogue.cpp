#include "catalogue.h"

#include <execution>
#include <numeric>
#include <iomanip>

namespace catalogue {
    std::ostream& operator<<(std::ostream& os, const BusStat& bus_info) {
        return os << "Bus " << bus_info.number << ": " << bus_info.stops_count << " stops on route, "
                  << bus_info.unique_stops_count << " unique stops, " << bus_info.route_length << " route length, "
                  << std::setprecision(6) << bus_info.curvature << " curvature";
    }

    size_t Bus::GetStopsCount() const {
        return (type == RouteType::CIRCLE) ? stops.size() : 2 * stops.size() - 1;
    }


    void TransportCatalogue::AddStop(Stop stop) {
        const auto position = stops_db_.insert(stops_db_.begin(), std::move(stop));
        stops_.insert({position->name, std::make_shared<Stop>(*position)});
        buses_stops_.insert({position->name, {}});
    }

    void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
        distances_stops_.insert({{stops_.at(stop_from), stops_.at(stop_to)}, distance});
    }

    void TransportCatalogue::AddBus(Bus bus) {
        for (auto& stop : bus.stops) {
            stop = stops_.find(stop)->first;
            UpdateMinMaxStopCoordinates(stops_.at(stop)->point);
        }
        bus.unique_stops = {bus.stops.begin(), bus.stops.end()};

        const auto position = buses_db_.insert(buses_db_.begin(), std::move(bus));
        buses_.insert({position->number, std::make_shared<Bus>(*position)});
        ordered_bus_list_.emplace(position->number);

        for (std::string_view stop : position->stops)
            buses_stops_[stop].insert(position->number);
    }

    std::optional<BusStat> TransportCatalogue::GetBusStat(std::string_view bus_number) const {
        if (buses_.count(bus_number) == 0)
            return std::nullopt;

        auto bus = buses_.at(bus_number);

        return BusStat{bus->number, bus->GetStopsCount(), bus->unique_stops.size(),
                       CalculateLength(buses_.at(bus_number)), CalculateGeoLength(buses_.at(bus_number))};
    }

    int TransportCatalogue::CalculateLength(const std::shared_ptr<Bus>& bus_info) const {
        auto get_route_length = [this](std::string_view from, std::string_view to) {
            auto key = std::make_pair(stops_.at(from), stops_.at(to));
            return (distances_stops_.count(key) > 0) ? distances_stops_.at(key) : distances_stops_.at({stops_.at(to), stops_.at(from)});
        };

        int forward_route = std::transform_reduce(bus_info->stops.begin(), std::prev(bus_info->stops.end()), std::next(bus_info->stops.begin()), 0, std::plus<>(), get_route_length);
        int backward_route = std::transform_reduce(bus_info->stops.rbegin(), std::prev(bus_info->stops.rend()), std::next(bus_info->stops.rbegin()), 0, std::plus<>(), get_route_length);

        return (bus_info->type == RouteType::CIRCLE) ? forward_route : forward_route + backward_route;
    }

    double TransportCatalogue::CalculateGeoLength(const std::shared_ptr<Bus>& bus_info) const {
        double geographic_length = std::transform_reduce(
            std::next(bus_info->stops.begin()), bus_info->stops.end(), bus_info->stops.begin(), 0.,
            std::plus<>(), [this](std::string_view from, std::string_view to) {
                return ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
            });

        return (bus_info->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.;
    }

    void TransportCatalogue::UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates) {
        coordinates_min_.lat = std::min(coordinates_min_.lat, coordinates.lat);
        coordinates_min_.lng = std::min(coordinates_min_.lng, coordinates.lng);
        coordinates_max_.lat = std::max(coordinates_max_.lat, coordinates.lat);
        coordinates_max_.lng = std::max(coordinates_max_.lng, coordinates.lng);
    }

    const geo::Coordinates& TransportCatalogue::GetMinStopCoordinates() const {
        return coordinates_min_;
    }

    const geo::Coordinates& TransportCatalogue::GetMaxStopCoordinates() const {
        return coordinates_max_;
    }

    const std::set<std::string_view>& TransportCatalogue::GetOrderedBusList() const {
        return ordered_bus_list_;
    }

    BusStopsStorage TransportCatalogue::GetFinalStops(std::string_view bus_name) const {
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

    BusStopsStorage TransportCatalogue::GetRouteInfo(std::string_view bus_name, bool include_backward_way) const {
        auto bus = buses_.at(bus_name);

        std::vector<std::shared_ptr<Stop>> stops;
        stops.reserve(bus->GetStopsCount());

        for (std::string_view stop : bus->stops)
            stops.emplace_back(stops_.at(stop));

        if (include_backward_way && bus->type == catalogue::RouteType::TWO_DIRECTIONAL) {
            for (auto stop = std::next(bus->stops.rbegin()); stop != bus->stops.rend(); ++stop)
                stops.emplace_back(stops_.at(*stop));
        }

        return std::make_pair(std::move(bus), std::move(stops));
    }

    StopsStorage TransportCatalogue::GetAllStopsFromRoutes() const {
        std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops;

        for (const auto& [_, bus] : buses_) {
            for (std::string_view stop : bus->stops) {
                if (stops.count(stop) == 0)
                    stops.emplace(stop, stops_.at(stop));
            }
        }

        return {stops.begin(), stops.end()};
    }

    std::unique_ptr<std::set<std::string_view>> TransportCatalogue::GetBusesPassingThroughTheStop(
        std::string_view stop_name) const {
        if (const auto position = buses_stops_.find(stop_name); position != buses_stops_.end())
            return std::make_unique<std::set<std::string_view>>(position->second);
        return nullptr;
    }
}  // namespace catalogue