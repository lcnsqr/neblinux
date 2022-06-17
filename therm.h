#ifndef Therm_h
#define Therm_h

#include "task.h"
#include "session.h"

class Therm: public Task {

  public:

  Therm(Session* session, int pin, unsigned long wait, unsigned int bufLen);
  ~Therm();

  void action();
  double celsiusPoly(double thermistor);
  double celsiusSteinhart(double thermistor);

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

  // Coeficientes do polinômio de terceira
  // ordem que aproxima a temperatura
  double pc[4];

  unsigned int bufLen;
  int* buf;
  unsigned int bufCount;
};

#endif
