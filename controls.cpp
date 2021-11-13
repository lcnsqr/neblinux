#include "task.h"
#include "controls.h"
#include <Arduino.h>

Controls::Controls(Session* session, int pin, unsigned long wait): Task(wait), session(session), pin(pin) {

  pinMode(pin, INPUT);
  // turn pullup resistor on
  digitalWrite(pin, HIGH);

}

void Controls::action(){
  wait = 40;
  if(digitalRead(pin) == LOW){
    session->on = ( session->on ) ? false : true;
    session->changed = true;
    wait = 500;
  }
}
