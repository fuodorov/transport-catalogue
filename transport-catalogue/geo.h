#pragma once

#include <cmath>

#include "constants.h"

namespace catalogue::geo {
    struct Coordinates {
        double lat, lng;
    };

    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace catalogue::constants;
        using namespace std;
        return EARTH_RADIUS * acos(sin(from.lat * DEG_TO_RAD) * sin(to.lat * DEG_TO_RAD) +
                                   cos(from.lat * DEG_TO_RAD) * cos(to.lat * DEG_TO_RAD) *
                                       cos(abs(from.lng - to.lng) * DEG_TO_RAD));
    }
}  // namespace catalogue::geo