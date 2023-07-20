#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
  double latitude{0.};
  double longitude{0.};

  bool operator==(const Coordinates& other) const {
    return latitude == other.latitude && longitude == other.longitude;
  }

  bool operator!=(const Coordinates& other) const { return !(*this == other); }
};

double CalculateDistance(Coordinates from, Coordinates to);

}  // namespace geo