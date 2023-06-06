#include <Arduino.h>
//#include <Wire.h>
#include <pthread.h>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib.h"
#include "queue.h"
#include "common.h"

// Pin definitions. The reference shows which GPIO have which number.
// The colors refer to the color of the cable attached to the pin.
#define STBY 27   // Green
#define CTL1 14   // Yellow
#define CTL2 12   // Yellow
#define PWM 13    // Red

// These are configurations for the pwm channel.
const int freq = 500;
const int pwmChannel = 0;
const int resolution = 8;

pthread_t move_thread;
pthread_t demo_thread;
int current_floor = 0; // Will only be written by move_thread, so doesn't need to be mutex'd.

// New requests will be put into either upQueue or downQueue, depending on the current location of the elevator.
// The elevator will satisfy all requests of one queue until it is empty, then empty the next one. If both are empty,
// the elevator will go into standby.
PriorityQueue* upQueue;
PriorityQueue* downQueue;
PriorityQueue* current_queue = NULL;


// With the default configuration, we accelerate over 2 seconds and decelerate over the same time. This means that whatever our top speed is 
// (in m/s), we move exactly that amount of m with accelerating and decelerating combined. So if our top speed is 2m/s, than accelerating takes
// 1m and decelerating takes 1m.
// 
// Lets assume that the stories are 4m apart floor-to-floor and that our elevators maximum speed is 2m/s (Note that we move only at 75%
// of the top speed, which in this case would be 2.5m/s). To move one story up, we would need to signal to the move thread to move 1 second
// between accelerating and decelerating. Or, in more general terms, whatever the distance between floors (y) or our speed, to move distance y,
// we need to move (y-(speed/s))/(speed/s) seconds ((4-2)/2 = 1)
const int story_height_cm = 400;
const int elevator_speed_cms = 200;  // This refers to the maximum speed the elevator will move, which is 75% of the top speed.

/*
 * Move thread will read this instruction to know what to do next. It is being written by the main thread.
 */
Instruction next = {.dir = CLOCKWISE, .time = 1.0};

/**
 * Accelerates from 0% to 75% in 2 seconds using the logistic curve function
 */ 
void accelerate();

/**
 * Decelerates from 75% to 0% in 2 seconds using the logistic curve function
 */
void decelerate();

/**
 * Sets the direction pins to clockwise or counterclockwise. Use with the direction enum
 */
void setDirection(direction dir);

/**
 * Thread function to move. Accelerates to 75%, stays at that for specified amount of time, and decelerates again.
 */
void* move(void *args) {
  setDirection(next.dir);
  digitalWrite(STBY, HIGH);
  accelerate();
  sleep(next.time);
  decelerate();
  digitalWrite(STBY, LOW);
  setDirection(NO_DIRECTION);
  return NULL;
}

/*
 * Thread function for demo purposes. Can specify a number of loops, and will simulate random requests after random amount of time.
 */
void *start_demo(void *number_of_loops) {
    srand(time(NULL));  
    int loops = (int) number_of_loops;
    Serial.printf("Demo thread started. Number of loops: %d\n", loops);
    int next_story = rand() % NUMBER_OF_FLOORS;
    for (int i = 0; i < loops; i++) {
      int sleep_amount = 5 + (rand() % 5);
      Serial.printf("Started iteration %d of %d. Will sleep %d seconds.\n", i, loops, sleep_amount);
      sleep(sleep_amount); // Sleep between 5 and 9 seconds
      int next_story = rand() % NUMBER_OF_FLOORS;
      if (next_story > current_floor) {
          insert(upQueue, next_story);
      } else if (next_story < current_floor) {
          insert(downQueue, next_story);
      }
    }
    return NULL;
}

/*
 * Standard Arduino function which contains the setup code.
 */
void setup() {
  //Wire.begin(); // Join I2C bus
  Serial.begin(9600);
  Serial.println("Starting setup...");
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  pinMode(STBY, OUTPUT);
  pinMode(CTL1, OUTPUT);
  pinMode(CTL2, OUTPUT);
  

  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWM, pwmChannel);

  // Initially, set all pins to 0 power
  ledcWrite(pwmChannel, 0);
  digitalWrite(STBY, LOW);
  setDirection(NO_DIRECTION);

  // Create the queues
  upQueue = createPriorityQueue(ASCENDING);
  downQueue = createPriorityQueue(DESCENDING);

  // Start demo thread.
  pthread_create(&demo_thread, NULL, start_demo, (void* ) 10);

  Serial.println("Finished setup!");
  digitalWrite(2, LOW);
}

/*
 * Standard Arduino function which contains the logic of the program. It just continually loops whatever is inside, as the name suggests.
 */
void loop() {
  Serial.println("Starting loop.");
  // Check if current_queue is NULL. If yes, check if one of the queues is not empty
  // Set the current_queue to the address of the non-empty queue. (upQueue Bias bc why not)
  if (current_queue == NULL) {
    if (upQueue->size != 0) {
      current_queue = upQueue;
    } else if (downQueue->size != 0) {
      current_queue = downQueue;
    }
  }
  // If the current queue is not empty, execute the next instruction.
  if (current_queue != NULL) {
    // Calculate the direction and the time the elevator needs to travel.
    int next_stop = extractNext(current_queue);
    float time = calculateTime(story_height_cm, elevator_speed_cms, abs(next_stop - current_floor));
    Serial.printf("Current story: %d. Next stop: %d, will take %f seconds.\n", current_floor, next_stop, time);
    direction dir = next_stop > current_floor ? CLOCKWISE : COUNTERCLOCKWISE;
    next = {.dir = dir, time = time};

    //Start the next thread.
    pthread_create(&move_thread, NULL, move, NULL);

    // Check for all buttons and enqueue if one is pressed.
    //checkButtons();

    // Wait for the thread to join and wait 2 seconds at the story.
    pthread_join(move_thread, NULL);
    current_floor = next_stop;
    Serial.printf("Arrived at floor no. %d. Waiting...", current_floor);
    sleep(2);

    // Check if current_queue is empty. If empty, set current_queue to NULL. Else, just repeat the loop.
    Serial.println("Checking current queue");
    if (current_queue->size == 0) {
      Serial.println("Current queue is empty.");
      current_queue = NULL;
    }
  } else {
    Serial.println("No requests yet :(");
    sleep(1); // Wait a second before checking again.
  }
}

void setDirection(direction dir) {
  digitalWrite(CTL1, (dir >> 1) & 1);
  digitalWrite(CTL2, dir & 1);
}

void accelerate() {
  int power = 0;
  for (int i = 0; i < 200; i++) {
    power = ceil(scurve(i, 192, -0.06, 100));
    ledcWrite(pwmChannel, power);
    delay(10);
  }
}

void decelerate() {
  int power = 192;
  for (int i = 0; i < 200; i++) {
    power = ceil(scurve(i, 192, 0.06, 100));
    ledcWrite(pwmChannel, power);
    delay(10);
  }
}