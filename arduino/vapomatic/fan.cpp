#include "task.h"
#include "fan.h"
#include <Arduino.h>

Fan::Fan(Session* session, unsigned long wait): Task(wait), session(session) {
  pinMode(session->settings.pFan, OUTPUT);

  digitalWrite(session->settings.pFan, LOW);
}

void Fan::action(){
  if ( 
    // Ativado
    (session->running() && ! session->dryrun) 
    // ou calibragem
    || session->calib ) digitalWrite(session->settings.pFan, HIGH);
  else digitalWrite(session->settings.pFan, LOW);

}
