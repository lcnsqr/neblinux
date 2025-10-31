#ifndef Rotary_h
#define Rotary_h

#include <Arduino.h>

// Globals moved to rotary.cpp; declare them here to avoid defining in the header.
extern volatile int lastEncoded;
extern long encoderMove;

// ISR prototype (implemented in rotary.cpp)
void rotaryEvent();

class Rotary {
public:
  Rotary() {
    // Encoder pins
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);

    // Rotary encoder helper variables are defined in rotary.cpp
    lastEncoded = 0;
    encoderMove = 0;

    // Attach interrupts using digitalPinToInterrupt for portability
    attachInterrupt(digitalPinToInterrupt(2), rotaryEvent, CHANGE);
    attachInterrupt(digitalPinToInterrupt(3), rotaryEvent, CHANGE);
  }
};

#endif
