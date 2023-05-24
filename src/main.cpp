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

// Accelerates from 0% to 75% in 2 seconds using the logistic curve function
void accelerate() {
  int power = 0;
  for (int i = 0; i < 200; i++) {
    power = ceil(scurve(i, 192, -0.06, 100));
    ledcWrite(pwmChannel, power);
    delay(10);
  }
}

// Decelerates from 75% to 0% in 2 seconds using the logistic curve function
void decelerate() {
  int power = 192;
  for (int i = 0; i < 200; i++) {
    power = ceil(scurve(i, 192, 0.06, 100));
    ledcWrite(pwmChannel, power);
    delay(10);
  }
}

// Thread function to move. Accelerates to 75%, stays at that for the specified time, and decelerates again.
void* move(void *time) {
  accelerate();
  //delay(*(int*) time);
  decelerate();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(STBY, OUTPUT);
  pinMode(CTL1, OUTPUT);
  pinMode(CTL2, OUTPUT);
  pinMode(2, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWM, pwmChannel);
  digitalWrite(STBY, LOW);
  digitalWrite(CTL1, HIGH);
  digitalWrite(CTL2, LOW);
}

void loop() {
  
  Serial.println("Beginning of loop!");
  digitalWrite(STBY, HIGH);
  delay(1000);
  digitalWrite(CTL1, LOW);
  digitalWrite(CTL2, HIGH);
  Serial.println("Creating first thread!");
  pthread_create(&thread, NULL, move, NULL);
  Serial.println("Created first thread!");
  digitalWrite(STBY, LOW);
  Serial.println("Joining first thread!");
  pthread_join(thread, NULL);
  Serial.println("Joined first thread!");
  digitalWrite(STBY, HIGH);
  Serial.println("We will probably not read this.");
  delay(1000);
  digitalWrite(CTL1, HIGH);
  digitalWrite(CTL2, LOW);
  pthread_create(&thread, NULL, move, NULL);
  digitalWrite(STBY, LOW);
  pthread_join(thread, NULL);
  Serial.println("Reached end of loop!");
}

double scurve(int x, int supremum, double k, int x0) {
  return supremum / (1 + pow(e, k * (x - x0)));
}