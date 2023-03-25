#pragma once

#include <algorithm>
#include <cmath>
#include <ostream>
#include <string>
#include <string_view>

#include "constants.h"

struct Coordinates {
    double lat, lng;

    static Coordinates ParseCoordinates(const std::string_view& string_coord, const char delimiter = ',') {
        Coordinates coord;
        size_t delimiter_pos = string_coord.find(delimiter);
        coord.lat = std::stod(std::string(string_coord.substr(0, delimiter_pos)));
        coord.lng = std::stod(std::string(string_coord.substr(delimiter_pos + 1)));
        return coord;
    }
};

inline bool operator== (const Coordinates& lhs, const Coordinates& rhs) {
    return abs(lhs.lat - rhs.lat) < constants::EPSILON && abs(lhs.lng - rhs.lng) < constants::EPSILON;
}

inline bool operator!= (const Coordinates& lhs, const Coordinates& rhs) {
    return !(lhs == rhs);
}

inline double ComputeDistance(Coordinates from, Coordinates to) {
    return std::acos(std::sin(from.lat * constants::DEG_TO_RAD) * std::sin(to.lat * constants::DEG_TO_RAD) +
                     std::cos(from.lat * constants::DEG_TO_RAD) * std::cos(to.lat * constants::DEG_TO_RAD) *
                     std::cos(std::abs(from.lng - to.lng) * constants::DEG_TO_RAD)) *
           constants::EARTH_RADIUS;
}
