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

  y[pts - 1] = session->tempEx - session->tempTarget;

  // Pontos correspondem às últimas leituras de temperatura.
  // Polinômio interpolador para regressão de segunda ordem.
  mat::leastsquares(pts, 2, x, y, session->shut);

  // Desligar se detectado crescimento
  // íngreme da distância temp - alvo.
  if (session->shut[0] > session->settings.shutLim[0] &&
      session->shut[1] > session->settings.shutLim[1]) {
    session->stop();
  }
}
