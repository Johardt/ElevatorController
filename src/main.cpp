#include <Arduino.h>
#include <pthread.h>
#include <cmath>
#include "lib.h"

#define STBY 14
#define CTL1 12
#define CTL2 13
#define PWM 21

const int freq = 500;
const int pwmChannel = 0;
const int resolution = 8;

pthread_t move_thread;

/*
 * Move thread will read this instruction to know what to do next. It is being written by the main thread.
 */
instruction next = {.direction = CLOCKWISE, .time = 1.0};

/**
 * Accelerates from 0% to 75% in 2 seconds using the logistic curve function
 */ 
void accelerate();

/**
 * Decelerates from 75% to 0% in 2 seconds using the logistic curve function
 */
void decelerate();

/*
 * Sets the direction pins to clockwise or counterclockwise. Use with 
 */
void setDirection(direction dir);

/**
 * Thread function to move. Accelerates to 75%, stays at that for specified amount of time, and decelerates again.
 */
void* move(void *args) {
  setDirection(next.direction);
  digitalWrite(STBY, HIGH);
  accelerate();
  sleep(next.time);
  decelerate();
  digitalWrite(STBY, LOW);
  setDirection(NO_DIRECTION);
  return NULL;
}

/*
 * Standard Arduino function which contains the setup code.
 */
void setup() {
  // put your setup code here, to run once:
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
  next = {.direction = CLOCKWISE, .time = 1.0};
  pthread_create(&move_thread, NULL, move, NULL);
  Serial.println("Elevator moving up");

  pthread_join(move_thread, NULL);
  Serial.println("Elevator reached first floor, waiting...");
  sleep(2);

  next = {.direction = COUNTERCLOCKWISE, .time = 1.0};
  pthread_create(&move_thread, NULL, move, NULL);
  Serial.println("Elevator moving down");

  pthread_join(move_thread, NULL);
  Serial.println("Elevator reached ground floor, waiting...");
  sleep(2);
}

double scurve(int x, int supremum, double k, int x0) {
  return supremum / (1 + pow(e, k * (x - x0)));
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