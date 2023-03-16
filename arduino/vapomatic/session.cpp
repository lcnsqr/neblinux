#include "session.h"
#include "mat.h"
#include <EEPROM.h>

Session::Session() {

  changed = false;
  state.tempCore = 0;
  state.tempEx = 0;
  state.tempTarget = 0;

  state.on = false;

  state.elapsed = 0;

  state.PID[0] = 0;
  state.PID[1] = 0;
  state.PID[2] = 0;
  state.PID[3] = 0;
  state.PID[4] = 0;

  state.shut[0] = 0;
  state.shut[1] = 0;
}

void Session::load() {
  // Carregar configurações salvas
  EEPROM.get(0, settings);

  // Temperatura alvo
  state.tempTarget = settings.tempTarget;

  // thCfs[0] : Coeficientes temperatura usados quando desativado
  state.thCfs[0][0] = 0;
  state.thCfs[0][1] = 1.0;
  state.thCfs[0][2] = 0;

  // Quantidade de pontos de calibragem
  // const int m = 3;
  // Grau do polinômio interpolador
  // const int n = 2;
  // thCfs[1] : Coeficientes usados quando ativado
  mat::leastsquares(3, 2, settings.tempCore, settings.tempEx, state.thCfs[1]);
}

void Session::save() { EEPROM.put(0, settings); }

void Session::reset() {
  // Temperaturas antes de calibrar
  settings.tempCore[0] = 81.16;
  settings.tempCore[1] = 105.49;
  settings.tempCore[2] = 152.40;
  settings.tempEx[0] = 150;
  settings.tempEx[1] = 180;
  settings.tempEx[2] = 200;

  settings.tempTarget = 180;

  // Coeficientes PID
  settings.PID[0] = 0.08;
  settings.PID[1] = 0.0007;
  settings.PID[2] = 0.06;

  // Desligamento automático
  settings.shutEnabled = 1;

  save();
  load();
}

void Session::start() {
  state.on = true;
  changed = true;
}

void Session::stop() {
  state.on = false;
  changed = true;

  // Resetar PID
  state.PID[0] = 0;
  state.PID[1] = 0;
  state.PID[2] = 0;
  state.PID[3] = 0;
  state.PID[4] = 0;

  // Lembrar da temperatura
  settings.tempTarget = state.tempTarget;
  save();
}

bool Session::running() { return state.on; }
