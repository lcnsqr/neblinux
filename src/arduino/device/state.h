#ifndef State_h
#define State_h

#include <stdint.h>

// Máximos para normalização dos domínios de temperatura e carga
#define TEMP_MAX 400.0
#define HEAT_MAX 255.0

// Marcação para checagem de transmissão serial
#define SERIAL_TAG 0x0f1e2d3c
// Comandos recebíveis via porta serial (char)
#define SERIAL_NONE 0x00
#define SERIAL_READ 0x10
#define SERIAL_WRITE 0x11
#define SERIAL_START 0x20
#define SERIAL_STOP 0x21
#define SERIAL_RESET 0x30
#define SERIAL_STORE 0x31

// Estrutura para o estado da sessão
struct State {
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

  // Coeficientes das retas de temperatura e carga para encerramento
  float sStop[2];

  // Limiares de encerramento
  float cStop[2];

  // On/off state change
  uint32_t on;

  // Fan load
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
  uint32_t tempStep;

  // timestamp da última mudança no target
  uint32_t targetLastChange;

  // Exibir splash screen
  uint32_t splash;

  // Descanso de tela (modo de espera)
  uint32_t screensaver;

  // Checagem de erro na transmissão serial
  uint32_t serialCheck;

};

// Estrutura para alteração de estado via porta serial
struct StateIO {
  // Temperatura alvo no exaustor
  float tempTarget;

  // Fan load
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
  // Limiares de parada
  float cStop[2];
  // Coeficientes das retas para temperatura e carga
  float sStop[2];

  // Rotary temperature step
  uint32_t tempStep;

  // Exibir splash screen
  uint32_t splash;

  // Standby (desncanso de tela)
  uint32_t screensaver;

  // Checagem de erro na transmissão serial
  uint32_t serialCheck;

};

#endif
