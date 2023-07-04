#include "geo.h"

namespace geo {

static const double dr = 3.1415926535 / 180.;
static const double R = 6371000;

double CalculateDistance(Coordinates from, Coordinates to) {
  using namespace std;
  return acos(sin(from.lat * dr) * sin(to.lat * dr) +
              cos(from.lat * dr) * cos(to.lat * dr) *
                  cos(abs(from.lng - to.lng) * dr)) *
         R;
}

} // namespace geo
