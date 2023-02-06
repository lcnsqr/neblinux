#include "shutdown.h"
#include "mat.h"
#include "session.h"
#include "task.h"
#include <Arduino.h>

Shutdown::Shutdown(Session *session, unsigned long wait)
    : Task(wait), session(session) {

  for (int i = 0; i < pts; ++i) {
    x[i] = (float)i;
    y[i] = 0;
  }
}

void Shutdown::action() {

  if (!(session->running() && session->settings.shutEnabled))
    return;

  for (int i = 1; i < pts; ++i)
    y[i - 1] = y[i];

  // Valor no atuador 0 - 255
  y[pts - 1] = session->PID[4];

  // Pontos correspondem às últimas leituras de temperatura.
  // Polinômio interpolador para regressão de primeira ordem.
  mat::leastsquares(pts, 1, x, y, session->shut);

  // Desligar se detectado queda íngreme na carga após 60s.
  if (session->shut[1] < slope && session->elapsed > minsec) {
    session->stop();
  }
}
