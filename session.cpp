#include "session.h"

Session::Session() {
  changed = true;
  temperature = 0;
  tempeTarget = 80;
  on = false;

  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 2e-0;
  PID[5] = 5e-3;
  PID[6] = 1e-0;
  PID[7] = 0;
}

void Session::cw(){
  tempeTarget++;
  changed = true;
}

void Session::ccw(){
  tempeTarget--;
  changed = true;
}

