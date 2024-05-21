#ifndef Therm_h
#define Therm_h

// Comandos recebíveis via porta serial (char)
#define SERIAL_NONE 0x00
#define SERIAL_READ 0x10

#include "max6675.h"
#include "task.h"

class Therm : public Task {

public:
  Therm(MAX6675 *thermocouple, unsigned long wait);

  MAX6675 *thermocouple;

  void action();

  // Temperatura em Celsius
  float celsius;

  // Requisição via porta serial
  // 0x00 → Nada
  // 0x10 → Requisição de estado da sessão
  uint8_t serialIn;
};

#endif
