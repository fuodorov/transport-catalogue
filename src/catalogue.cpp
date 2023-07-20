#include "catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::add_stop(Stop &&stop) {
    stops.push_back(std::move(stop));
    Stop *stop_buf = &stops.back();
    stopname_to_stop.insert(transport_catalogue::StopMap::value_type(stop_buf->name, stop_buf));
}

void TransportCatalogue::add_bus(Bus &&bus) {
    Bus *bus_buf;

    buses.push_back(std::move(bus));
    bus_buf = &buses.back();
    busname_to_bus.insert(BusMap::value_type(bus_buf->name, bus_buf));

    for (Stop *stop : bus_buf->stops) {
        stop->buses.push_back(bus_buf);
    }

    bus_buf->route_length = get_distance_to_bus(bus_buf);
}

void TransportCatalogue::add_distance(const std::vector<Distance> &distances) {
    for (auto distance : distances) {
        auto dist_pair = std::make_pair(distance.start, distance.end);
        distance_to_stop.insert(DistanceMap::value_type(dist_pair, distance.distance));
    }
}

Bus *TransportCatalogue::get_bus(std::string_view bus_name) {
    if (busname_to_bus.empty()) {
        return nullptr;
    }

    if (busname_to_bus.count(bus_name)) {
        return busname_to_bus.at(bus_name);
    } else {
        return nullptr;
    }
}

Stop *TransportCatalogue::get_stop(std::string_view stop_name) {
    if (stopname_to_stop.empty()) {
        return nullptr;
    }

    if (stopname_to_stop.count(stop_name)) {
        return stopname_to_stop.at(stop_name);
    } else {
        return nullptr;
    }
}

std::deque<Stop> TransportCatalogue::get_stops() const {
    return stops;
}

std::deque<Bus> TransportCatalogue::get_buses() const {
    return buses;
}

BusMap TransportCatalogue::get_busname_to_bus() const {
    return busname_to_bus;
}

StopMap TransportCatalogue::get_stopname_to_stop() const {
    return stopname_to_stop;
}

std::unordered_set<const Stop *> TransportCatalogue::get_uniq_stops(Bus *bus) {
    std::unordered_set<const Stop *> unique_stops;
    unique_stops.insert(bus->stops.begin(), bus->stops.end());

    return unique_stops;
}

double TransportCatalogue::get_length(Bus *bus) {
    return transform_reduce(
        next(bus->stops.begin()), bus->stops.end(), bus->stops.begin(), 0.0, std::plus<>{},
        [](const Stop *lhs, const Stop *rhs) {
            return geo::CalculateDistance({(*lhs).latitude, (*lhs).longitude}, {(*rhs).latitude, (*rhs).longitude});
        });
}

std::unordered_set<const Bus *> TransportCatalogue::stop_get_uniq_buses(Stop *stop) {
    std::unordered_set<const Bus *> unique_stops;
    unique_stops.insert(stop->buses.begin(), stop->buses.end());

    return unique_stops;
}

DistanceMap TransportCatalogue::get_distance() const {
    return distance_to_stop;
}

size_t TransportCatalogue::get_distance_stop(const Stop *begin, const Stop *finish) const {
    if (distance_to_stop.empty()) {
        return 0;

    } else {
        if (const auto &stop_ptr_pair = std::make_pair(begin, finish); distance_to_stop.count(stop_ptr_pair)) {
            return distance_to_stop.at(stop_ptr_pair);

        } else if (const auto &stop_ptr_pair = std::make_pair(finish, begin); distance_to_stop.count(stop_ptr_pair)) {
            return distance_to_stop.at(stop_ptr_pair);

        } else {
            return 0;
        }
    }
}

size_t TransportCatalogue::get_distance_to_bus(Bus *bus) {
    size_t distance = 0;
    auto stops_size = bus->stops.size() - 1;

    for (int i = 0; i < static_cast<int>(stops_size); i++) {
        distance += get_distance_stop(bus->stops[i], bus->stops[i + 1]);
    }

    return distance;
}

}  // end namespace transport_catalogue