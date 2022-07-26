#include "task.h"
#include "fan.h"
#include <Arduino.h>

Fan::Fan(Session* session, int pin, unsigned long wait): Task(wait), session(session), pin(pin) {
  pinMode(pin, OUTPUT);

  digitalWrite(pin, LOW);
}

void Fan::action(){
  if ( session->on && ! session->dryrun ) digitalWrite(pin, HIGH);
  else digitalWrite(pin, LOW);
}
