#ifndef Therm_h
#define Therm_h

#include "max6675.h"
#include "task.h"

class Therm : public Task {

public:
  Therm(MAX6675 *thermocouple, unsigned long wait);

  MAX6675 *thermocouple;

  void action();

  // Intervalo de transmissão serial
  static const float serial_wait = 250;
  long int serial_before, serial_now;
};

#endif
