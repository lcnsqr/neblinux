#include "session.h"

Session::Session() {
  dryrun = 0;

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
  PID[4] = 3e-6;
  PID[5] = 1e-5;
  PID[6] = 3e-1;
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

