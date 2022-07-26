#include "task.h"
#include "controls.h"
#include <Arduino.h>

Controls::Controls(Session* session, int pin, unsigned long wait): Task(wait), session(session), pin(pin) {

  pinMode(pin, INPUT);
  // turn pullup resistor on
  digitalWrite(pin, HIGH);

  changed = 0;
  repeat = 2000;

}

void Controls::action(){

  if(digitalRead(pin) == LOW && ! session->on){

    // Evitar mudança imediata de estado
    if ( millis() - changed > repeat ) changed = millis();
    else return;

    session->on = true;
    session->changed = true;
  }

  if(digitalRead(pin) == LOW && session->on){

    // Evitar mudança imediata de estado
    if ( millis() - changed > repeat ) changed = millis();
    else return;

    session->on = false;
    session->changed = true;
  }

}
