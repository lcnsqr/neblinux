#include "heater.h"
#include "task.h"
#include <Arduino.h>

Heater::Heater(int port, Session *session, unsigned long wait)
    : port(port), Task(wait), session(session) {
  pinMode(port, OUTPUT);

  /*
  c0 = &(session->state.settings.PID[0]);
  c1 = &(session->state.settings.PID[1]);
  c2 = &(session->state.settings.PID[2]);
  P = &(session->state.PID[0]);
  I = &(session->state.PID[1]);
  D = &(session->state.PID[2]);
  dif_old = &(session->state.PID[3]);
  F = &(session->state.PID[4]);
  */
}

void Heater::action() {
  if (session->running()) {
    // Aquecer
    float dif = (float)wait * (session->state.tempTarget - session->state.tempEx);

    session->state.PID[0] = session->settings.PID[0] * dif;
    session->state.PID[1] = session->state.PID[1] + session->settings.PID[1] * dif;
    // Resfriamento passivo
    if (session->state.PID[1] < 0)
      session->state.PID[1] = 0;
    session->state.PID[2] = session->settings.PID[2] * (dif - session->state.PID[3]);

    session->state.PID[4] = session->state.PID[0] + session->state.PID[1] + session->state.PID[2];
    if (session->state.PID[4] < 0)
      session->state.PID[4] = 0;
    if (session->state.PID[4] > 255)
      session->state.PID[4] = 255;

    analogWrite(port, (int)session->state.PID[4]);

    session->state.PID[3] = dif;

    return;
  }

  // Apenas zerar
  analogWrite(port, 0);
}
