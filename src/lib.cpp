#include "lib.h"
#include <cmath>

const double e = 2.718281828459;

double scurve(int x, int supremum, double k, int x0) {
  return supremum / (1 + pow(e, k * (x - x0)));
}

float calculateTime(int story_height_cm, int elevator_speed_cms, int amount_of_stories) {
    if (amount_of_stories == 0) {
      return 0;
    } 
    float totalHeight = story_height_cm * amount_of_stories;
    return (totalHeight - elevator_speed_cms) / elevator_speed_cms;
}