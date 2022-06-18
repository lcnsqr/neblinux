#include "session.h"

Session::Session() {
  changed = true;
  temperature = 0;
  tempeTarget = 50;

  on = false;

  tempeGapTherm = 130;

  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 9.5e-3;
  PID[5] = 4.5e-4;
  PID[6] = 9.5e-2;
  PID[7] = 0;
}

void Session::cw(){
  tempeTarget += 10;
  changed = true;
}

void Session::ccw(){
  tempeTarget -= 10;
  changed = true;
}

