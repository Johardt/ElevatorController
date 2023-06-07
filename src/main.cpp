#include <Arduino.h>
#include <Wire.h>
#include <pthread.h>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib.h"
#include "queue.h"
#include "config.h"
#include "i2c.h"

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

// pthread_t threadA;
// pthread_t threadB;
pthread_t observer;

int current_floor = 0; // Will only be written by main loop, so doesn't need to be mutex'd.

// New requests will be put into either upQueue or downQueue, depending on the current location of the elevator.
// The elevator will satisfy all requests of one queue until it is empty, then empty the next one. If both are empty,
// the elevator will go into standby.
PriorityQueue* upQueue;
PriorityQueue* downQueue;
PriorityQueue* current_queue = NULL;

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

// void* threadAFunc(void *args) {
//   while (true) {
//     writeOutputA(0b10000000);
//     delay(10);
//   }
// }

// void* threadBFunc(void *args) {
//   while (true) {
//     writeOutputA(0b01000000);
//     delay(13);
//   }
// }

void* observerFunc(void *args) {
  int val;
  while(true) {
    val = readInputB();
    Serial.printf("Got input %d from register B!", val);
    delay(10);
  }
  return NULL;
}

/**
 * Thread function to move. Accelerates to 75%, stays at that for specified amount of time, and decelerates again.
 */
void* move(void *args) {
  setDirection(next.dir);
  digitalWrite(STBY, HIGH);
  // writeOutputA(0b10000000);
  accelerate();
  delay(next.time * 1000);
  decelerate();
  digitalWrite(STBY, LOW);
  // writeOutputA(0b00000000);
  setDirection(NO_DIRECTION);
  return NULL;
}

/*
 * Thread function for demo purposes. Can specify a number of loops, and will simulate random requests after random amount of time.
 */
void *start_demo(void *number_of_loops) {
    // srand(time(NULL));  
    int loops = (int) number_of_loops;
    Serial.printf("Demo thread started. Number of loops: %d\n", loops);
    int next_floor = rand() % NUMBER_OF_FLOORS;
    for (int i = 0; i < loops; i++) {
      int sleep_amount = (rand() % 5);
      int next_floor = rand() % NUMBER_OF_FLOORS;
      Serial.printf("Started iteration %d of %d. Will sleep %d seconds. Next floor: %d\n", i, loops, sleep_amount, next_floor);
      delay(sleep_amount * 1000); // Sleep between 5 and 9 seconds
      if (next_floor > current_floor) {
          insert(upQueue, next_floor);
      } else if (next_floor < current_floor) {
          insert(downQueue, next_floor);
      }
    }
    return NULL;
}

/*
 * Standard Arduino function which contains the setup code.
 */
void setup() {
  setupI2Cpins();
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
  pthread_create(&demo_thread, NULL, start_demo, (void* ) 100);

  // pthread_create(&threadA, NULL, threadAFunc, NULL);
  // pthread_create(&threadB, NULL, threadBFunc, NULL);
  pthread_create(&observer, NULL, observerFunc, NULL);

  Serial.println("Finished setup!");
  digitalWrite(2, LOW);
}

/*
 * Standard Arduino function which contains the logic of the program. It just continually loops whatever is inside, as the name suggests.
 */
void loop() {
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
    if (next_stop != -100) {
      float time = calculateTime(STORY_HEIGHT_CM, ELEVATOR_SPEED_CMS, abs(next_stop - current_floor));
      Serial.printf("Current story: %d. Next stop: %d, will take %f seconds.\n", current_floor, next_stop, time);
      direction dir = next_stop > current_floor ? CLOCKWISE : COUNTERCLOCKWISE;
      next = {.dir = dir, time = time};
      current_floor = next_stop;

      //Start the next thread.
      pthread_create(&move_thread, NULL, move, NULL);

      // Check for all buttons and enqueue if one is pressed.
      // checkButtons();

      // Wait for the thread to join and wait 2 seconds at the story.
      pthread_join(move_thread, NULL);
      Serial.printf("Arrived at floor no. %d. Waiting...\n", current_floor);
      delay(2000);
    }
    // Check if current_queue is empty. If empty, set current_queue to NULL. Else, just repeat the loop.
    Serial.println("Checking current queue");
    if (current_queue->size == 0) {
      Serial.println("Current queue is empty.");
      current_queue = NULL;
    }
  } else {
    Serial.println("No requests yet :(");
    delay(1000); // Wait a second before checking again.
    // Check for all buttons and enqueue if one is pressed.
    // checkButtons();
  }
}

void setDirection(direction dir) {
  digitalWrite(CTL1, dir & 2);
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