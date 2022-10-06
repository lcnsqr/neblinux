#include "task.h"
#include "heater.h"
#include <Arduino.h>

Heater::Heater(Session* session, unsigned long wait): Task(wait), session(session) {
  pinMode(session->settings.pHeater, OUTPUT);

  /*
  c0 = &(session->settings.PID[0]);
  c1 = &(session->settings.PID[1]);
  c2 = &(session->settings.PID[2]);
  P = &(session->PID[0]);
  I = &(session->PID[1]);
  D = &(session->PID[2]);
  dif_old = &(session->PID[3]);
  F = &(session->PID[4]);
  */
}

void Heater::action(){
  if ( ! session->running() || session->dryrun ){
    analogWrite(session->settings.pHeater, 0);
    return;
  }
  
  float dif = (float)wait * (session->tempTarget - session->tempEx);

  session->PID[0] = session->settings.PID[0] * dif;
  session->PID[1] = session->PID[1] + session->settings.PID[1] * dif;
  // Resfriamento passivo
  if ( session->PID[1] < 0 ) session->PID[1] = 0;
  session->PID[2] = session->settings.PID[2] * (dif - session->PID[3]);

  session->PID[4] = session->PID[0] + session->PID[1] + session->PID[2];
  if ( session->PID[4] < 0 ) session->PID[4] = 0;
  if ( session->PID[4] > 255 ) session->PID[4] = 255;

  analogWrite(session->settings.pHeater, (int)session->PID[4]);

  session->PID[3] = dif;
}
