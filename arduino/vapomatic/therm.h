#ifndef Therm_h
#define Therm_h

#include "session.h"
#include "task.h"

class Therm : public Task {

public:
  Therm(Session *session, int port, unsigned long wait);

  void action();
  float celsiusPoly(float thermistor);
  float celsiusSteinhart(float thermistor);

  // R1 Value
  static const float r1 = 95000.0;

  // Thermistor nominal value
  static const float thermNominal = 100000.0;
  static const float bCoef = 3950.0;
  static const float tempNominal = 25.0;

  static const int bufLen = 5;
  int buf[bufLen];
  unsigned int bufCount;

private:
  Session *session;

  // Analog in
  int port;

};

#endif
