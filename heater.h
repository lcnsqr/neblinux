#ifndef Heater_h
#define Heater_h

#include "task.h"
#include "session.h"

class Heater: public Task {

  public:

  Heater(Session* session, int pin, unsigned long wait);

  void action();

  double setpoint;
  double input;

  private:

  // PWM output
  int pin;

  Session* session;

  double y[3];
  int yPos;
};

#endif
