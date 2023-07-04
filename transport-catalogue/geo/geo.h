#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
  double lat{0.};
  double lng{0.};
};

double CalculateDistance(Coordinates from, Coordinates to);

} // namespace geo