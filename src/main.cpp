#include <Arduino.h>
#include <pthread.h>
#include <cmath>
#include "lib.h"

#define STBY 14
#define CTL1 12
#define CTL2 13
#define PWM 21

#define CLOCKWISE 2         // 10 in binary, so CTL1 1, CTL2 0
#define COUNTERCLOCKWISE 1  // 01 in binary, so CTL1 0, CTL2 1

const int freq = 500;
const int pwmChannel = 0;
const int resolution = 8;

pthread_t thread;
instruction next = {.direction = CLOCKWISE, .time = 1.0};

/**
 * Accelerates from 0% to 75% in 2 seconds using the logistic curve function
 */ 
void accelerate();

/**
 * Decelerates from 75% to 0% in 2 seconds using the logistic curve function
 */
void decelerate();

void sequence();

void setDirection(int direction);

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
  setDirection(0);
  return NULL;
}

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
  setDirection(0);
}

void loop() {
  Serial.println("Starting sequence");
  sequence();
}

void sequence() {
  // Put motor controller in standby
  next = {.direction = CLOCKWISE, .time = 1.0};
  pthread_create(&thread, NULL, move, NULL);
  Serial.println("Elevator moving up");

  pthread_join(thread, NULL);
  Serial.println("Elevator reached first floor, waiting...");
  sleep(2);

  next = {.direction = COUNTERCLOCKWISE, .time = 1.0};
  pthread_create(&thread, NULL, move, NULL);
  Serial.println("Elevator moving down");

  pthread_join(thread, NULL);
  Serial.println("Elevator reached ground floor, waiting...");
  sleep(2);
}

void setDirection(int direction) {
  digitalWrite(CTL1, (direction >> 1) & 1);
  digitalWrite(CTL2, direction & 1);
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