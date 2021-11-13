#include "task.h"
#include "therm.h"
#include <math.h>
#include <Arduino.h>

Therm::Therm(Session* session, int pin, unsigned long wait): Task(wait), session(session), pin(pin) {
  r1 = 9950.0;

  thermNominal = 10000.0;
  bCoef = 3950.0;
  tempNominal =  25.0;

  analogReference(EXTERNAL);
}

void Therm::action(){
  session->temperature = celsius(analogRead(pin));
}

double Therm::celsius(int reading){
  double thermistor = (double)reading;
  double r0 = r1 / ((1023.0 / thermistor) - 1.0);

  double steinhart;
  steinhart = r0 / thermNominal;
  steinhart = log(steinhart);
  steinhart /= bCoef;
  steinhart += 1.0 / (tempNominal + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  return steinhart;
}
