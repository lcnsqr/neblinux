#include "task.h"
#include "heater.h"
#include <Arduino.h>

Heater::Heater(Session* session, int pin, unsigned long wait): Task(wait), session(session), pin(pin) {
  pinMode(pin, OUTPUT);
}

void Heater::action(){
  if ( ! session->on ){
    analogWrite(5, 0);
    return;
  }

  analogWrite(5, (int)session->tempeTarget);
}
