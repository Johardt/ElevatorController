typedef struct INSTRUCTION {
    int direction;
    float time;
} instruction;

/**
 * s-curve function. given an x value, returns y of the [Logistic function](https://en.wikipedia.org/wiki/Logistic_function)
 */
double scurve(int x, int supremum, double k, int x0);