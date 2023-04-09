#include "catalogue.h"

#include <execution>
#include <numeric>
#include <iomanip>

namespace catalogue {
    std::ostream& operator<<(std::ostream& os, const BusStatistics& bus_info) {
        os << "Bus " << bus_info.number << ": " << bus_info.stops_count << " stops on route, "
        << bus_info.unique_stops_count << " unique stops, ";
        os << bus_info.rout_length << " route length, ";
        os << std::setprecision(6) << bus_info.curvature << " curvature";
        return os;
    }

    size_t Bus::GetStopsCount() const {
        return (type == RouteType::CIRCLE) ? stop_names.size() : 2 * stop_names.size() - 1;
    }


    void TransportCatalogue::AddStop(Stop stop) {
        const auto position = stops_storage_.insert(stops_storage_.begin(), std::move(stop));
        stops_.insert({position->name, std::make_shared<Stop>(*position)});
        buses_through_stop_.insert({position->name, {}});
    }

    void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
        distances_between_stops_.insert({{stops_.at(stop_from), stops_.at(stop_to)}, distance});
    }

    void TransportCatalogue::AddBus(Bus bus) {
        for (auto& stop : bus.stop_names) {
            stop = stops_.find(stop)->first;
            UpdateMinMaxStopCoordinates(stops_.at(stop)->point);
        }
        bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

        const auto position = buses_storage_.insert(buses_storage_.begin(), std::move(bus));
        buses_.insert({position->number, std::make_shared<Bus>(*position)});
        ordered_bus_list_.emplace(position->number);

        for (std::string_view stop : position->stop_names)
            buses_through_stop_[stop].insert(position->number);
    }

    std::optional<BusStatistics> TransportCatalogue::GetBusStatistics(std::string_view bus_number) const {
        if (buses_.count(bus_number) == 0)
            return std::nullopt;

        auto bus_info = buses_.at(bus_number);

        BusStatistics result;
        result.number = bus_info->number;
        result.stops_count = bus_info->GetStopsCount();
        result.unique_stops_count = bus_info->unique_stops.size();
        result.rout_length = CalculateRouteLength(bus_info);
        result.curvature = static_cast<double>(result.rout_length) / CalculateGeographicLength(bus_info);

        return result;
    }

    int TransportCatalogue::CalculateRouteLength(const std::shared_ptr<Bus>& bus_info) const {
        auto get_route_length = [this](std::string_view from, std::string_view to) {
            auto key = std::make_pair(stops_.at(from), stops_.at(to));
            return (distances_between_stops_.count(key) > 0)
                    ? distances_between_stops_.at(key)
                    : distances_between_stops_.at({stops_.at(to), stops_.at(from)});
        };

        int forward_route =
            std::transform_reduce(bus_info->stop_names.begin(), std::prev(bus_info->stop_names.end()),
                                std::next(bus_info->stop_names.begin()), 0, std::plus<>(), get_route_length);
        if (bus_info->type == RouteType::CIRCLE)
            return forward_route;

        int backward_route =
            std::transform_reduce(bus_info->stop_names.rbegin(), std::prev(bus_info->stop_names.rend()),
                                std::next(bus_info->stop_names.rbegin()), 0, std::plus<>(), get_route_length);

        return forward_route + backward_route;
    }

    double TransportCatalogue::CalculateGeographicLength(const std::shared_ptr<Bus>& bus_info) const {
        double geographic_length = std::transform_reduce(
            std::next(bus_info->stop_names.begin()), bus_info->stop_names.end(), bus_info->stop_names.begin(), 0.,
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

        if (bus->stop_names.empty())
            return std::make_pair(bus, stops);

        if (bus->type == RouteType::CIRCLE) {
            stops.emplace_back(stops_.at(bus->stop_names.front()));
        } else if (bus->type == RouteType::TWO_DIRECTIONAL) {
            stops.emplace_back(stops_.at(bus->stop_names.front()));

            if (bus->stop_names.front() != bus->stop_names.back())
                stops.emplace_back(stops_.at(bus->stop_names.back()));
        }

        return std::make_pair(bus, stops);
    }

    BusStopsStorage TransportCatalogue::GetRouteInfo(std::string_view bus_name, bool include_backward_way) const {
        auto bus = buses_.at(bus_name);

        std::vector<std::shared_ptr<Stop>> stops;
        stops.reserve(bus->GetStopsCount());

        for (std::string_view stop : bus->stop_names)
            stops.emplace_back(stops_.at(stop));

        if (include_backward_way && bus->type == catalogue::RouteType::TWO_DIRECTIONAL) {
            for (auto stop = std::next(bus->stop_names.rbegin()); stop != bus->stop_names.rend(); ++stop)
                stops.emplace_back(stops_.at(*stop));
        }

        return std::make_pair(std::move(bus), std::move(stops));
    }

    StopsStorage TransportCatalogue::GetAllStopsFromRoutes() const {
        std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops;

        for (const auto& [_, bus] : buses_) {
            for (std::string_view stop : bus->stop_names) {
                if (stops.count(stop) == 0)
                    stops.emplace(stop, stops_.at(stop));
            }
        }

        return {stops.begin(), stops.end()};
    }

    std::unique_ptr<std::set<std::string_view>> TransportCatalogue::GetBusesPassingThroughTheStop(
        std::string_view stop_name) const {
        if (const auto position = buses_through_stop_.find(stop_name); position != buses_through_stop_.end())
            return std::make_unique<std::set<std::string_view>>(position->second);
        return nullptr;
    }
}  // namespace catalogue