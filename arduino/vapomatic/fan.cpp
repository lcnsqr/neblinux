#include "fan.h"
#include "task.h"
#include <Arduino.h>

Fan::Fan(int port, Session *session, unsigned long wait)
    : port(port), Task(wait), session(session) {
  pinMode(port, OUTPUT);

  analogWrite(port, 0);
}

void Fan::action() {
  // Ativado
  if (session->running()) {
    analogWrite(port, session->state.fan);
    return;
  }

  analogWrite(port, 0);
}
