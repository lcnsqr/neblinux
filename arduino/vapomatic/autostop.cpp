#include "autostop.h"
#include "mat.h"
#include "session.h"
#include "task.h"
#include <Arduino.h>

Autostop::Autostop(Session *session, unsigned long wait)
    : Task(wait), session(session) {

  for (int i = 0; i < pts; ++i) {
    x[i] = (float)i;
    y[i] = 0;
  }
}

void Autostop::action() {

  if (!(session->running() && session->state.autostop))
    return;

  for (int i = 1; i < pts; ++i)
    y[i - 1] = y[i];

  // Valor no atuador 0 - 255
  y[pts - 1] = session->state.PID[4];

  // Pontos correspondem às últimas leituras de temperatura.
  // Polinômio interpolador para regressão de primeira ordem.
  mat::leastsquares(pts, 1, x, y, session->state.cStop);

  // Desligar se detectado queda íngreme na carga após 60s.
  if (session->state.cStop[1] < slope && session->state.elapsed > minsec) {
    session->stop();
  }
}
