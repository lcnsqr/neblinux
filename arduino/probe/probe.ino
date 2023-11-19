#include "max6675.h"
#include "task.h"
#include "therm.h"

// Loop de serviços
Tasks tasks;

static const int thermoSO = 12;
static const int thermoCS = 10;
static const int thermoCSK = 13;
MAX6675 thermocouple(thermoCSK, thermoCS, thermoSO);
// Leitura da temperatura no termopar
Therm therm(&thermocouple, 250);

void setup() {
  Serial.begin(9600);

  // Serviços (alterar quantidade em task.h)
  tasks.tasks[0] = &therm;
}

void loop() { tasks.run(); }
