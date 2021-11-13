#include "session.h"

Session::Session() {
  changed = true;
  temperature = 0;
  tempeTarget = 0;
  on = false;
}

void Session::cw(){
  tempeTarget++;
  changed = true;
}

void Session::ccw(){
  tempeTarget--;
  changed = true;
}

