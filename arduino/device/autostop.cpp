#include "autostop.h"
#include "session.h"
#include "state.h"
#include "task.h"
#include <Arduino.h>

Autostop::Autostop(Session *session, unsigned long wait)
    : Task(wait), session(session) {

  // Zerar leituras
  for (int i = 0; i < pts; ++i) {
    tempEx[i] = 0;
    heat[i] = 0;
  }

  // Índice inicial das leituras
  iy = 0;

  // Zerar RHS
  s[0] = 0;
  s[1] = 0;

  // Constante de decaimento
  decay = -log(0.4);
}

void Autostop::action() {

  tempEx[iy] = session->state.tempEx / TEMP_MAX;
  heat[iy] = session->state.PID[4] / HEAT_MAX;
  iy = (iy + 1) % pts;

  // Calcular coeficiente de reta para temperatura
  s[0] = tempEx[iy] + tempEx[(iy + 1) % pts] + tempEx[(iy + 2) % pts] + tempEx[(iy + 3) % pts];
  s[1] = x0 * tempEx[iy] + x1 * tempEx[(iy + 1) % pts] + x2 * tempEx[(iy + 2) % pts] + x3 * tempEx[(iy + 3) % pts];
  session->state.sStop[0] = ((s[1] / a2) - (s[0] / a0)) / ((a3 / a2) - (a1 / a0));

  // Calcular coeficiente de reta para a carga
  s[0] = heat[iy] + heat[(iy + 1) % pts] + heat[(iy + 2) % pts] + heat[(iy + 3) % pts];
  s[1] = x0 * heat[iy] + x1 * heat[(iy + 1) % pts] + x2 * heat[(iy + 2) % pts] + x3 * heat[(iy + 3) % pts];
  session->state.sStop[1] = ((s[1] / a2) - (s[0] / a0)) / ((a3 / a2) - (a1 / a0));

  // Fator de decaimento dos limiares de parada depende do tempo decorrido
  float g = exp(-decay * ((float)(session->state.elapsed) - (float)minSec) / (float)minSec);

  // CRITÉRIOS PARA PARADA AUTOMÁTICA:
  // Ativada na configuração
  // elpsed > minSec
  // PID ativo
  // tempEx superior a tempTarget
  // Reta recente da temperatura superior a cStop[0] (ponderada pelo decaimento
  // temporal) Reta recente da carga inferior a cStop[1] (ponderada pelo
  // decaimento temporal) now - targetLastChange > targetMinSec

  if (session->state.autostop && session->state.elapsed > minSec &&
      session->state.PID_enabled &&
      session->state.tempEx > session->state.tempTarget &&
      session->state.sStop[0] > g * session->state.cStop[0] &&
      session->state.sStop[1] < g * session->state.cStop[1] &&
      (millis() / 1000) - session->state.targetLastChange > targetMinSec)
    session->stop();
}
