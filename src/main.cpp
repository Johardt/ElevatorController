#include <Arduino.h>
#include <Wire.h>
#include <pthread.h>
#include <cmath>
#include "lib.h"

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
int current_story = 0; // Will only be written by move_thread, so doesn't need to be mutex'd.

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
instruction next = {.dir = CLOCKWISE, .time = 1.0};

// New requests will be put into either upQueue or downQueue, depending on the current location of the elevator.
// The elevator will satisfy all requests of one queue until it is empty, then empty the next one. If both are empty,
// the elevator will go into standby.
priority_queue upQueue;
priority_queue downQueue;
priority_queue* current_queue = NULL;

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
  current_story += (next.dir = CLOCKWISE) ? 1 : -1;
  setDirection(NO_DIRECTION);
  return NULL;
}

/*
 * Standard Arduino function which contains the setup code.
 */
void setup() {
  Wire.begin(); // Join I2C bus
  Serial.begin(9600);

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
}

/*
 * Standard Arduino function which contains the logic of the program. It just continually loops whatever is inside, as the name suggests.
 */
void loop() {
  // Check if current_queue is NULL. If yes, check if one of the queues is not empty
  // Set the current_queue to the address of the non-empty queue.

  // Start the next thread.

  // Check for all buttons and enqueue if one is pressed.

  // Wait for the thread to join and wait 2 seconds at the story.

  // Check if current_queue is empty. If empty, set current_queue to NULL. Else, just repeat the loop.
  next = {.dir = CLOCKWISE, .time = 1.0};
  pthread_create(&move_thread, NULL, move, NULL);
  Serial.println("Elevator moving up");

  pthread_join(move_thread, NULL);
  Serial.println("Elevator reached first floor, waiting...");
  sleep(2);

  next = {.dir = COUNTERCLOCKWISE, .time = 1.0};
  pthread_create(&move_thread, NULL, move, NULL);
  Serial.println("Elevator moving down");

  pthread_join(move_thread, NULL);
  Serial.println("Elevator reached ground floor, waiting...");
  sleep(2);
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