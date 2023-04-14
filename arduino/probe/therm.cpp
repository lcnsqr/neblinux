#include "therm.h"
#include "task.h"
#include <Arduino.h>
#include "max6675.h"

Therm::Therm(MAX6675 *thermocouple, unsigned long wait)
    : Task(wait), thermocouple(thermocouple) {

    }

void Therm::action() {
  // Comunicação serial
  serial_now = millis();
  if (serial_now - serial_before >= serial_wait) {
    // Enviar temperatura para o utilitário de setup
    float temp = thermocouple->readCelsius();
    Serial.write((char*)&temp, sizeof(float));
    serial_before = serial_now;
  }

}
