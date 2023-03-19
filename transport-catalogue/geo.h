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

    static Coordinates ParseFromStringView(const std::string_view& string_coord, const char delimiter = ',') {
        size_t comma_pos = string_coord.find_first_of(delimiter);
        Coordinates coord;
        coord.lat = std::stod(std::string(string_coord.substr(0, comma_pos)));
        coord.lng = std::stod(std::string(string_coord.substr(comma_pos + 1)));
        return coord;
    }
};

inline bool operator== (const Coordinates& lhs, const Coordinates& rhs) {
    return abs(lhs.lat - rhs.lat) < EPSILON && abs(lhs.lng - rhs.lng) < EPSILON;
}

inline bool operator!= (const Coordinates& lhs, const Coordinates& rhs) {
    return !(lhs == rhs);
}

inline std::ostream& operator<< (std::ostream& out, const Coordinates& coord) {
    return out << "<|" << coord.lat << ", " << coord.lng << "|>";
}

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    return acos(sin(from.lat * DEG_TO_RAD) * sin(to.lat * DEG_TO_RAD) + cos(from.lat * DEG_TO_RAD) * cos(to.lat * DEG_TO_RAD) * cos(abs(from.lng - to.lng) * DEG_TO_RAD)) * EARTH_RADIUS;
}
