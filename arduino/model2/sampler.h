#ifndef Sampler_h
#define Sampler_h

#include "task.h"
#include "therm.h"

class Sampler: public Task {

  public:

  Sampler(Therm* th, int pin, unsigned long wait);

  void action();

  private:

  Therm* th;

  // Analog in
  int pin;

};

#endif
