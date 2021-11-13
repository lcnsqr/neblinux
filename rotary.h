#ifndef Rotary_h
#define Rotary_h

// Rotary encoder helper variable
volatile int lastEncoded;

// Encoder up/down integer
long int encoderMove;

// Function called by interrupt
void rotaryEvent(){
  int MSB = digitalRead(2); //MSB = most significant bit
  int LSB = digitalRead(3); //LSB = least significant bit
  
  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderMove++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderMove--;
  lastEncoded = encoded; //store this value for next time
} 

class Rotary {
  public:
  Rotary() {

    // Encoder pins
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);

    // Rotary encoder helper variables
    lastEncoded = 0;
    encoderMove = 0;

    // Call rotaryEvent() when any high/low changed seen
    // on interrupt 0 (pin 2), or interrupt 1 (pin 3)
    attachInterrupt(0, rotaryEvent, CHANGE);
    attachInterrupt(1, rotaryEvent, CHANGE);

  }
};

#endif
