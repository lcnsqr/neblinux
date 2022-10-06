#include "task.h"
#include "fan.h"
#include <Arduino.h>

Fan::Fan(int port, Session* session, unsigned long wait): port(port), Task(wait), session(session) {
  pinMode(port, OUTPUT);

  digitalWrite(port, LOW);
}

void Fan::action(){
  // Ativado
  if ( session->running() ){
    digitalWrite(port, HIGH);
    return;
  }

  // Calibrando
  if ( session->calib ){
    digitalWrite(port, HIGH);
    return;
  }

  digitalWrite(port, LOW);

}
