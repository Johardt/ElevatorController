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
 * Using the default s-curve configuration (2 second accelerating, 2 second decelerating) and given story height and elevator
 * speed, calculates the time (in seconds) the elevator needs to move at full speed to reach the desired story.
 */
float calculateTime(int story_height_cm, int elevator_speed_cms, int amount_of_stories);

/**
 * Pseudo-method for now. Checks all buttons in the building, and processes if they have been pressed.
 */
void checkButtons();