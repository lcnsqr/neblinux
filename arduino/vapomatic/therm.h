#ifndef Therm_h
#define Therm_h

#include "task.h"
#include "session.h"

class Therm: public Task {

  public:

  Therm(Session* session, int pin, unsigned long wait, unsigned int bufLen);
  ~Therm();

  void action();
  float celsiusPoly(float thermistor);
  float celsiusSteinhart(float thermistor);

  private:

  Session* session;

  // Analog in
  int pin;

  // R1 Value
  float r1;

  // Thermistor nominal value
  float thermNominal;
  float bCoef;
  float tempNominal;

  unsigned int bufLen;
  int* buf;
  unsigned int bufCount;
};

#endif
