#ifndef Heater_h
#define Heater_h

#include "task.h"
#include "session.h"

class Heater: public Task {

  public:

  Heater(Session* session, int pin, unsigned long wait);

  void action();

  private:

  // PWM output
  int pin;

  Session* session;
};

#endif
