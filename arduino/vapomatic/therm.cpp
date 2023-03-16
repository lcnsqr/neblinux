#include "therm.h"
#include "task.h"
#include <Arduino.h>
#include <math.h>
#include <stdlib.h>

Therm::Therm(Session *session, int port, unsigned long wait)
    : Task(wait), session(session), port(port) {}

void Therm::action() {
  int bufSum = 0;
  if (bufCount == bufLen) {
    bufCount = 0;
    for (int i = 0; i < bufLen; ++i)
      bufSum += buf[i];
    session->state.analogTherm = (float)bufSum / (float)bufLen;
    session->state.tempCore = celsiusSteinhart(session->state.analogTherm);
    session->state.tempEx = celsiusPoly(session->state.tempCore);
  } else {
    buf[bufCount++] = analogRead(port);
  }
}

float Therm::celsiusPoly(float core) {
  const int i = (int)session->running();
  return session->state.thCfs[i][0] + session->state.thCfs[i][1] * core +
         session->state.thCfs[i][2] * pow(core, 2);
}

float Therm::celsiusSteinhart(float thermistor) {
  float r0 = r1 / ((1023.0 / thermistor) - 1.0);

  float steinhart;
  steinhart = r0 / thermNominal;
  steinhart = log(steinhart);
  steinhart /= bCoef;
  steinhart += 1.0 / (tempNominal + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  return steinhart;
}
