typedef enum {
    NO_DIRECTION,
    COUNTERCLOCKWISE,
    CLOCKWISE
} direction;

typedef struct INSTR {
    direction dir;
    float time;
} Instruction;

/**
 * s-curve function. given an x value, returns y of the [Logistic function](https://en.wikipedia.org/wiki/Logistic_function).
 */
double scurve(int x, int supremum, double k, int x0);

/**
 * With the default configuration, we accelerate over 2 seconds and decelerate over the same time. This means that whatever our top speed is 
 * (in m/s), we move exactly that amount of m with accelerating and decelerating combined. So if our top speed is 2m/s, than accelerating takes
 * 1m and decelerating takes 1m.

 * Lets assume that the stories are 4m apart floor-to-floor and that our elevators maximum speed is 2m/s (Note that we move only at 75%
 * of the top speed, which in this case would be 2.5m/s). To move one story up, we would need to signal to the move thread to move 1 second
 * between accelerating and decelerating. Or, in more general terms, whatever the distance between floors (y) or our speed, to move distance y,
 * we need to move (y-(speed/s))/(speed/s) seconds ((4-2)/2 = 1)
 */
float calculateTime(int story_height_cm, int elevator_speed_cms, int amount_of_stories);

/**
 * Pseudo-method for now. Checks all buttons in the building, and processes if they have been pressed.
 */
void checkButtons();