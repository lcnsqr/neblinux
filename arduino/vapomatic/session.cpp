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

  state.PID[0] = 0;
  state.PID[1] = 0;
  state.PID[2] = 0;
  state.PID[3] = 0;
  state.PID[4] = 0;
  state.PID[5] = 1;

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

}

void Session::save() { EEPROM.put(0, settings); }

void Session::reset() {
  // Coeficientes do polinômio grau 3 que infere temperatura

  // Coeficientes da temperatura antes de calibrar
  settings.cTemp[0] = -96.484375;
  settings.cTemp[1] =   5.983643;
  settings.cTemp[2] =  -0.053619;
  settings.cTemp[3] =   0.000196;

  settings.tempTarget = 180;

  // Coeficientes PID
  settings.cPID[0] = 0.08;
  settings.cPID[1] = 0.0007;
  settings.cPID[2] = 0.06;

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
