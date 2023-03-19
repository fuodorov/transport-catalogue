#include <iomanip>
#include <numeric>
#include <utility>

#include "transport_catalogue.h"

namespace transport_catalogue {

    namespace stop_catalogue {

        using namespace detail;

        std::ostream& operator<<(std::ostream& out, const BusesToStopNames& buses) {
            bool first = true;
            for (const std::string_view& bus : buses) {
                if (!first) {
                    out << std::string(" ");
                }
                out << bus;
                first = false;
            }
            return out;
        }

        const Stop* Catalogue::Push(std::string&& name, std::string&& string_coord) {
            stops_.push_back({ std::move(name), Coordinates::ParseFromStringView(string_coord) });
            stop_buses_.insert({ &stops_.back(), {} });
            return &stops_.back();
        }

        void Catalogue::PushBusToStop(const Stop* stop, const std::string_view& bus_name) {
            stop_buses_.at(stop).insert(bus_name);
        }

        void Catalogue::AddDistance(const Stop* stop_1, const Stop* stop_2, double distance) {
            PointerPair<Stop> stop_pair_direct = { stop_1, stop_2 };
            PointerPair<Stop> stop_pair_reverse = { stop_2, stop_1 };

            distances_between_stops_[stop_pair_direct] = distance;

            if (distances_between_stops_.count(stop_pair_reverse) == 0) {
                distances_between_stops_[stop_pair_reverse] = distance;
            }
        }

    }

    namespace bus_catalogue {

        using namespace detail;
        using namespace stop_catalogue;

        Bus::Bus(std::string&& name, std::deque<const Stop*>&& route, double route_lenght, double route_true_lenght, RouteType route_type)
            : name(name)
            , route(route)
            , route_type(route_type)
            , route_lenght(route_lenght)
            , route_true_lenght(route_true_lenght)
            , stops_on_route(route.size()) {
            std::unordered_set<std::string_view> unique_stops_names;
            for (const Stop* stop : route) {
                unique_stops_names.insert(stop->name);
            }
            unique_stops = unique_stops_names.size();

            if (route_type == RouteType::BackAndForth) {
                stops_on_route = stops_on_route * 2 - 1;
            }
        }

        std::ostream& operator<<(std::ostream& out, const Bus& bus) {
            static const char* str_bus = "Bus ";
            static const char* str_sep = ": ";
            static const char* str_comma = ", ";
            static const char* str_space = " ";
            static const char* str_stops_on_route = "stops on routee";
            static const char* str_unique_stops = "unique stops";
            static const char* str_routee_lenght = "routee length";
            static const char* str_curvature = "curvature";

            double curvature = bus.route_true_lenght / bus.route_lenght;

            out << str_bus << bus.name << str_sep;
            out << bus.stops_on_route << str_space << str_stops_on_route << str_comma;
            out << bus.unique_stops << str_space << str_unique_stops << str_comma;
            out << std::setprecision(6) << bus.route_true_lenght << str_space << str_routee_lenght << str_comma;
            out << std::setprecision(6) << curvature << str_space << str_curvature;

            return out;
        }

        const Bus* Catalogue::Push(std::string&& name, std::vector<std::string_view>&& string_route, RouteType type,
            const VirtualCatalogue<Stop>& stops_catalogue, const DistancesContainer& stops_distances) {
            std::deque<const Stop*> stops;
            for (const std::string_view& stop_name : string_route) {
                auto [it, res] = stops_catalogue.At(stop_name);
                if (res) {
                    stops.push_back((*it).second);
                }
            }
            double route_geo_lenght = CalcRouteGeoLenght(stops, type);
            double route_true_lenght = CalcRouteTrueLenght(stops, stops_distances, type);
            buses_.push_back(Bus(std::move(name), std::move(stops), route_geo_lenght, route_true_lenght, type));
            return &buses_.back();
        }

        double Catalogue::CalcRouteGeoLenght(const std::deque<const Stop*>& route, RouteType route_type) {
            std::vector<double> distance(route.size() - 1);
            std::transform(
                route.begin(), route.end() - 1,
                route.begin() + 1, distance.begin(),
                [](const Stop* from, const Stop* to) {
                    return ComputeDistance(from->coord, to->coord);
                });
            double lenght = std::reduce(distance.begin(), distance.end());

            if (route_type == RouteType::BackAndForth) {
                lenght *= 2.0;
            }

            return lenght;
        }

        double Catalogue::CalcRouteTrueLenght(const std::deque<const Stop*>& route, const DistancesContainer& stops_distances, RouteType route_type) {
            std::vector<double> distance(route.size() - 1);
            std::transform(
                route.begin(), route.end() - 1,
                route.begin() + 1, distance.begin(),
                [&stops_distances](const Stop* from, const Stop* to) {
                    return stops_distances.at(PointerPair<Stop>{ from, to });
                });
            double lenght = std::reduce(distance.begin(), distance.end());

            if (route_type == RouteType::BackAndForth) {
                std::transform(
                    route.rbegin(), route.rend() - 1,
                    route.rbegin() + 1, distance.begin(),
                    [&stops_distances](const Stop* from, const Stop* to) {
                        return stops_distances.at(PointerPair<Stop>{ from, to });
                    });
                lenght += std::reduce(distance.begin(), distance.end());
            }

            return lenght;
        }

    }

}
