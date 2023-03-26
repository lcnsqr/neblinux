#ifndef Therm_h
#define Therm_h

#include "task.h"
#include "max6675.h"

class Therm : public Task {

public:
  Therm(MAX6675 *thermocouple, unsigned long wait);

  MAX6675 *thermocouple;

  void action();

};

#endif

