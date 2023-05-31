#include "lib.h"
#include <cmath>

const double e = 2.718281828459;

double scurve(int x, int supremum, double k, int x0) {
  return supremum / (1 + pow(e, k * (x - x0)));
}