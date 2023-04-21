#include "session.h"
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

  state.autostop = 1;
  state.sStop[0] = 0;
  state.sStop[1] = 0;
  
  state.ts = 0;

  state.tempStep = 10;

  state.targetLastChange = 0;

  state.splash = 1;
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

  // Desligamento automático
  state.autostop = settings.autostop;
  state.cStop[0] = settings.cStop[0];
  state.cStop[1] = settings.cStop[1];

  // Passo do giro
  state.tempStep = settings.tempStep;
}

void Session::save() {
  // Temperatura alvo
  settings.tempTarget = state.tempTarget;

  // Desligamento automático
  settings.autostop = state.autostop;
  settings.cStop[0] = state.cStop[0];
  settings.cStop[1] = state.cStop[1];

  // Passo do giro
  settings.tempStep = state.tempStep;

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
  state.cPID[0] = 0.04;
  state.cPID[1] = 0.0001;
  state.cPID[2] = 0.06;

  // Desligamento automático
  state.autostop = 1;

  // Limiares de parada a um minuto
  state.cStop[0] = 0.01;
  state.cStop[1] = -0.075;

  // Passo do giro
  state.tempStep = 10;

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

  // Resetar coeficientes das retas de temperatura e carga
  state.sStop[0] = 0;
  state.sStop[1] = 0;

  // Lembrar da temperatura
  settings.tempTarget = state.tempTarget;
  save();
}

bool Session::running() { return state.on; }
