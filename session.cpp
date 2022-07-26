#include "session.h"

Session::Session() {
  dryrun = 0;

  tempeMin = 20.0;
  tempeMax = 240.0;

  changed = true;
  tempeCore = 0;
  tempeEx = 0;
  tempeTarget = 180;

  on = false;

  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 9.5e-3;
  PID[5] = 1e-4;
  PID[6] = 9.5e-2;
  PID[7] = 0;
}

void Session::cw(){
  if ( tempeTarget + 10 > tempeMax ) return;
  tempeTarget += 10;
  changed = true;
}

void Session::ccw(){
  if ( tempeTarget - 10 < tempeMin ) return;
  tempeTarget -= 10;
  changed = true;
}

