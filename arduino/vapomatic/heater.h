#ifndef Heater_h
#define Heater_h

#include "task.h"
#include "session.h"

class Heater: public Task {

  public:

  Heater(Session* session, unsigned long wait);

  void action();

  private:

  Session* session;

  /*
  float* P;
  float* I;
  float* D;
  float* dif_old;
  float* c0;
  float* c1;
  float* c2;
  float* F;
  */

};

#endif
