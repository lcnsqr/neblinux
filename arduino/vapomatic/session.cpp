#include "session.h"

Session::Session() {
  dryrun = 1;

  tempeMin = 20.0;
  tempeMax = 240.0;

  changed = false;
  tempeCore = 0;
  tempeEx = 0;
  tempeTarget = 180;

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

void Session::load(struct Settings* st){
  settings = st;

  // thCfs[0] : Coeficientes usados quando desativado
  thCfs[0][0] = 0;
  thCfs[0][1] = 1.0;
  thCfs[0][2] = 0;

  // thCfs[1] : Coeficientes usados quando ativado
  thCfs[1][0] = 0;
  thCfs[1][1] = 1.9375;
  thCfs[1][2] = -33.125;

  // Coeficientes PID
  settings->PID[0] = 1e-2;
  settings->PID[1] = 1.5e-4;
  settings->PID[2] = 7e-2;

  // Limiares de desligamento
  settings->shutLim[0] = 4.0;
  settings->shutLim[1] = 1.0;
}

void Session::start(){
  on = true;
  changed = true;
}

void Session::stop(){
  on = false;
  changed = true;

  // Resetar PID
  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 0;
}

bool Session::running(){
  return on;
}
