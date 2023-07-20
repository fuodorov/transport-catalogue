#include "geo.h"

namespace geo {

static const double dr = 3.1415926535 / 180.;
static const double R = 6371000;

double CalculateDistance(Coordinates from, Coordinates to) {
  using namespace std;
  return acos(sin(from.latitude * dr) * sin(to.latitude * dr) +
              cos(from.latitude * dr) * cos(to.latitude * dr) *
                  cos(abs(from.longitude - to.longitude) * dr)) *
         R;
}

}  // namespace geo
