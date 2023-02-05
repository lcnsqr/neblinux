#include "session.h"
#include "mat.h"
#include <EEPROM.h>

Session::Session() {

  changed = false;
  tempCore = 0;
  tempEx = 0;
  tempTarget = 0;

  on = false;

  elapsed = 0;

  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 0;

  shut[0] = 0;
  shut[1] = 0;
}

void Session::load() {
  // Carregar configurações salvas
  EEPROM.get(0, settings);

  // Temperatura alvo
  tempTarget = settings.tempTarget;

  // thCfs[0] : Coeficientes temperatura usados quando desativado
  thCfs[0][0] = 0;
  thCfs[0][1] = 1.0;
  thCfs[0][2] = 0;

  // Quantidade de pontos de calibragem
  // const int m = 3;
  // Grau do polinômio interpolador
  // const int n = 2;
  // thCfs[1] : Coeficientes usados quando ativado
  mat::leastsquares(3, 2, settings.tempCore, settings.tempEx, thCfs[1]);
}

void Session::save() { EEPROM.put(0, settings); }

void Session::reset() {
  // Temperaturas antes de calibrar
  settings.tempCore[0] = 81.16;
  settings.tempCore[1] = 105.49;
  settings.tempCore[2] = 152.40;
  settings.tempEx[0] = 140;
  settings.tempEx[1] = 180;
  settings.tempEx[2] = 220;

  settings.tempTarget = 180;

  // Coeficientes PID
  settings.PID[0] = 0.08;
  settings.PID[1] = 0.0008;
  settings.PID[2] = 0;

  // Limiares de desligamento
  settings.shutLim[0] = 1.0;
  settings.shutLim[1] = 1.0;
  settings.shutEnabled = 1;

  save();
  load();
}

void Session::start() {
  on = true;
  changed = true;
}

void Session::stop() {
  on = false;
  changed = true;

  // Resetar PID
  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 0;

  // Lembrar da temperatura
  settings.tempTarget = tempTarget;
  save();
}

bool Session::running() { return on; }
