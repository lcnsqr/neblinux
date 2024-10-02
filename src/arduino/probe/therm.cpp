#include "therm.h"
#include "max6675.h"
#include "task.h"
#include <Arduino.h>

Therm::Therm(MAX6675 *thermocouple, unsigned long wait)
    : Task(wait), thermocouple(thermocouple) {}

void Therm::action() {
  // Receber comando via porta serial
  if ( serialIn == SERIAL_NONE ) {
    if (Serial.available() > 0 ){
      Serial.readBytes((char *)&(serialIn), 1);
    }
  }

  if ( serialIn == SERIAL_READ ) {

    // Enviar temperatura para o utilitário de setup
    celsius = thermocouple->readCelsius();
    Serial.write((char *)&celsius, sizeof(float));

  }
  // Liberar nova chegada de comando
  serialIn = SERIAL_NONE;

}
