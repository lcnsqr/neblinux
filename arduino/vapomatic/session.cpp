#include "session.h"
#include "mat.h"
#include <EEPROM.h>

Session::Session() {

  changed = false;

  state.header = 0xffff;

  state.tempCore = 0;
  state.tempEx = 0;
  state.tempTarget = 0;

  state.on = false;
  state.fan = false;

  state.elapsed = 0;

  state.PID_enabled = 1;

  state.PID[0] = 0;
  state.PID[1] = 0;
  state.PID[2] = 0;
  state.PID[3] = 0;
  state.PID[4] = 0;

  state.shut[0] = 0;
  state.shut[1] = 0;
  
  state.ts = 0;
}

void Session::load() {
  // Carregar configurações salvas
  EEPROM.get(0, settings);

  // Temperatura alvo
  state.tempTarget = settings.tempTarget;

  // Coeficientes do polinômio grau 3 que infere temperatura
  for (int i = 0; i <= 3; i++)
    state.cTemp[i] = settings.cTemp[i];

  // Coeficientes de ponderação do PID
  for (int i = 0; i < 3; i++)
    state.cPID[i] = settings.cPID[i];

}

void Session::save() {
  // Temperatura alvo
  settings.tempTarget = state.tempTarget;

  // Desligamento automático
  //settings.shutEnabled = state.shutEnabled;

  // Coeficientes do polinômio grau 3 que infere temperatura
  for (int i = 0; i <= 3; i++)
    settings.cTemp[i] = state.cTemp[i];

  // Coeficientes de ponderação do PID
  for (int i = 0; i < 3; i++)
    settings.cPID[i] = state.cPID[i];

  EEPROM.put(0, settings);
}

void Session::reset() {
  // Coeficientes do polinômio grau 3 que infere temperatura
  state.cTemp[0] = -56.339844;
  state.cTemp[1] =   4.008545;
  state.cTemp[2] =  -0.020069;
  state.cTemp[3] =   0.000046;

  state.tempTarget = 180;

  // Coeficientes PID
  state.cPID[0] = 0.06;
  state.cPID[1] = 0.0007;
  state.cPID[2] = 0.06;

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
