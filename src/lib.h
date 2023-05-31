typedef enum {
    NO_DIRECTION,
    COUNTERCLOCKWISE,
    CLOCKWISE
} direction;

typedef struct INSTR {
    direction dir;
    float time;
} instruction;

typedef struct {
} priority_queue;

/**
 * s-curve function. given an x value, returns y of the [Logistic function](https://en.wikipedia.org/wiki/Logistic_function)
 */
double scurve(int x, int supremum, double k, int x0);