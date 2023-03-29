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

  // Coeficientes do polinômio de segunda
  // ordem que estima a temperatura na saída
  // thCfs[0] : Coeficientes usados quando desativado
  // thCfs[1] : Coeficientes usados quando ativado
  float thCfs[2][3];

  // Indicador de encerramento
  float shut[2];

  // On/off state change
  uint32_t on;

  // Independent fan control
  uint32_t fan;

  // 0,1,2: PID
  // 3: D anterior (atual - alvo)
  // 4: Valor do atuador
  // 5: Se 0, aplicar atuador mas não calcular PID
  float PID[6];

  // timestamp
  uint32_t ts;
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
};

#endif
