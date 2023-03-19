#pragma once

#include <algorithm>
#include <cmath>
#include <ostream>
#include <string>
#include <string_view>

constexpr double PI = 3.14159265358979323846;
constexpr double EARTH_RADIUS = 6371000.0;
constexpr double EPSILON = 1e-6;
constexpr double DEG_TO_RAD = PI / 180.0;

struct Coordinates {
    double lat = 0;
    double lng = 0;

    static Coordinates ParseFromStringView(const std::string_view& string_coord, const std::string& delimiter = ",") {
        Coordinates coord;
        size_t pos = string_coord.find(delimiter);
        coord.lat = std::stod(std::string(string_coord.substr(0, pos)));
        coord.lng = std::stod(std::string(string_coord.substr(pos + 1)));
        return coord;
    }
};

inline bool operator== (const Coordinates& lhs, const Coordinates& rhs) {
    return std::abs(lhs.lat - rhs.lat) < EPSILON && std::abs(lhs.lng - rhs.lng) < EPSILON;
}

inline bool operator!= (const Coordinates& lhs, const Coordinates& rhs) {
    return !(lhs == rhs);
}

inline std::ostream& operator<< (std::ostream& out, const Coordinates& coord) {
    using namespace std::string_literals;
    out << "<|"s << coord.lat << ", "s << coord.lng << "|>"s;
    return out;
}

inline double ComputeDistance(Coordinates from, Coordinates to) {
    return EARTH_RADIUS * acos(sin(from.lat * DEG_TO_RAD) * sin(to.lat * DEG_TO_RAD) + cos(from.lat * DEG_TO_RAD) * cos(to.lat * DEG_TO_RAD) * cos(abs(from.lng - to.lng) * DEG_TO_RAD));
}
