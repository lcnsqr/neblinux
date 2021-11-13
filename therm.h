#ifndef Therm_h
#define Therm_h

#include "task.h"
#include "session.h"

class Therm: public Task {

  public:

  Therm(Session* session, int pin, unsigned long wait);

  void action();
  double celsius(int reading);

  private:

  Session* session;

  // Analog in
  int pin;

  // R1 Value
  double r1;

  // Thermistor nominal value
  double thermNominal;
  double bCoef;
  double tempNominal;

};

#endif
