typedef enum {
    NO_DIRECTION,
    COUNTERCLOCKWISE,
    CLOCKWISE
} direction;

typedef struct INSTRUCTION {
    direction direction;
    float time;
} instruction;

const double e = 2.718281828459;

/**
 * s-curve function. given an x value, returns y of the [Logistic function](https://en.wikipedia.org/wiki/Logistic_function)
 */
double scurve(int x, int supremum, double k, int x0);