#include "session.h"

Session::Session() {
  dryrun = 0;

  screen = 0;

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
  PID[4] = 1e-2;
  PID[5] = 1e-4;
  PID[6] = 7e-2;
  PID[7] = 0;

  end[0] = 0;
  end[1] = 0;
}

void Session::cw(){
  if ( tempeTarget + 10 > tempeMax ) return;
  tempeTarget += 10;
}

void Session::ccw(){
  if ( tempeTarget - 10 < tempeMin ) return;
  tempeTarget -= 10;
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
  PID[7] = 0;
}

bool Session::running(){
  return on;
}
