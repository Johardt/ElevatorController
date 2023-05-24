#include <Arduino.h>
#include <pthread.h>
#include <cmath>

#define STBY 14
#define CTL1 12
#define CTL2 13
#define PWM 21

const int freq = 500;
const int pwmChannel = 0;
const int resolution = 8;

const double e = 2.718281828459;

pthread_t thread;

/**
 * s-curve function. given an x value, returns y of the [Logistic function](https://en.wikipedia.org/wiki/Logistic_function)
 */
double scurve(int x, int supremum, double k, int x0);

/**
 * Accelerates from 0% to 75% in 2 seconds using the logistic curve function
 */ 
void accelerate() {
  int power = 0;
  for (int i = 0; i < 200; i++) {
    power = ceil(scurve(i, 192, -0.06, 100));
    ledcWrite(pwmChannel, power);
    delay(10);
  }
}

/**
 * Decelerates from 75% to 0% in 2 seconds using the logistic curve function
 */
void decelerate() {
  int power = 192;
  for (int i = 0; i < 200; i++) {
    power = ceil(scurve(i, 192, 0.06, 100));
    ledcWrite(pwmChannel, power);
    delay(10);
  }
}

/**
 * Thread function to move. Accelerates to 75%, stays at that for specified amount of time, and decelerates again.
 */
void* move(void *args) {
  accelerate();
  //delay(*(int*) time);
  decelerate();
  return NULL;
}

void sequence();

/*
 * CW and CCW
 */
void setCW();
void setCCW();

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
  digitalWrite(CTL1, LOW);
  digitalWrite(CTL2, LOW);
}

void loop() {
  Serial.println("Starting sequence");
  sequence();
}

double scurve(int x, int supremum, double k, int x0) {
  return supremum / (1 + pow(e, k * (x - x0)));
}

void sequence() {
  // Put motor controller in standby
  digitalWrite(CTL1, HIGH);
  digitalWrite(CTL2, LOW);
  digitalWrite(STBY, HIGH);
  pthread_create(&thread, NULL, move, NULL);
  Serial.println("Elevator moving up");

  pthread_join(thread, NULL);
  Serial.println("Elevator reached first floor, waiting...");
  digitalWrite(CTL1, LOW);
  digitalWrite(STBY, LOW);
  sleep(2);

  digitalWrite(CTL2, HIGH);
  digitalWrite(STBY, HIGH);
  pthread_create(&thread, NULL, move, NULL);
  Serial.println("Elevator moving down");

  pthread_join(thread, NULL);
  Serial.println("Elevator reached ground floor, waiting...");
  digitalWrite(CTL2, LOW);
  digitalWrite(STBY, LOW);
  sleep(2);
}