#include "i2c.h"
#include <Arduino.h>
#include <Wire.h>
#include <pthread.h>

pthread_mutex_t lock;

void setupI2Cpins() {
    pthread_mutex_init(&lock, NULL);
    Wire.begin(); // Join I2C bus
    Wire.beginTransmission(0x20);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();
}

void writeOutputA(int output) {
    //pthread_mutex_lock(&lock);
    Wire.beginTransmission(0x20);
    Wire.write(0x12); // address port A
    Wire.write(output);
    Wire.endTransmission();
    //pthread_mutex_unlock(&lock);
}

int readInputB() {
    int value1;
    int value2;
    Wire.beginTransmission(0x20);
    Wire.requestFrom(0x20, 2);
    int nrOfBytes = Wire.available();
    if (nrOfBytes == 2) {
        value1 = Wire.read();
        value2 = Wire.read();
        Serial.println(value1);
        Serial.println(value2);
    }
    return value1;
}