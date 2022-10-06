#include "task.h"
#include "fan.h"
#include <Arduino.h>

Fan::Fan(Session* session, unsigned long wait): Task(wait), session(session) {
  pinMode(session->settings.pFan, OUTPUT);

  digitalWrite(session->settings.pFan, LOW);
}

void Fan::action(){
  if ( session->running() && ! session->dryrun ) digitalWrite(session->settings.pFan, HIGH);
  else digitalWrite(session->settings.pFan, LOW);
}
