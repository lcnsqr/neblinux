#include "therm.h"
#include "task.h"
#include <Arduino.h>
#include "max6675.h"

Therm::Therm(MAX6675 *thermocouple, unsigned long wait)
    : Task(wait), thermocouple(thermocouple) {

    }

void Therm::action() {
  float temp = thermocouple->readCelsius();
  Serial.write((char*)&temp, sizeof(float));
  delay(100);
}
