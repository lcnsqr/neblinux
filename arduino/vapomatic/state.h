#ifndef State_h
#define State_h

#include <stdint.h>

// Estrutura para o estado da sessão
struct State {
  // Marca inicial do dataframe para transmissão serial
  int32_t header;

  // Tempo em segundos em atividade
  int32_t elapsed;

  // Temperatura na base da resistência
  float tempCore;
  // Temperatura no exaustor
  float tempEx;
  // Temperatura alvo no exaustor
  float tempTarget;
  // Leitura ADC do termistor
  float analogTherm;

  // Coeficientes do polinômio grau 3 que infere temperatura
  float cTemp[4];

  // Parar automaticamente
  uint32_t autostop;

  // Indicador de encerramento
  float cStop[2];

  // On/off state change
  uint32_t on;

  // Independent fan control
  uint32_t fan;

  // Coeficientes de ponderação do PID
  float cPID[3];

  // 0,1,2: Parcelas do PID
  // 3: D anterior (atual - alvo)
  // 4: Valor do atuador
  float PID[5];

  // Ligar/desligar PID
  uint32_t PID_enabled;

  // timestamp
  uint32_t ts;


  // Rotary temperature step
  int tempStep;
};

// Estrutura de comando via porta serial
struct StateIO {
  // Temperatura alvo no exaustor
  float tempTarget;

  // On/off state change
  uint32_t on;

  // Independent fan control
  uint32_t fan;

  // Ligar/desligar PID
  uint32_t PID_enabled;

  // Valor no atuador
  uint32_t heat;

  // Coeficientes do polinômio grau 3 que infere temperatura
  float cTemp[4];

  // Coeficientes de ponderação do PID
  float cPID[3];

  // Parar automaticamente
  uint32_t autostop;

  // Armazenar definições na EEPROM
  uint32_t store;

};

#endif
