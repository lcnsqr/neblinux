#ifndef Session_h
#define Session_h

#include "state.h"

// Estrutura para armazenar na EEPROM
struct Settings {
  float cTemp[4];     // Coeficientes do polinômio grau 3 que infere temperatura
  float cPID[3];      // Coeficientes PID
  bool autostop;  // Desligamento automático ativo/inativo
  float cStop[2];      // Limiares de parada dos coeficientes de reta para temperatura e carga
  float tempTarget;  // Temperatura alvo padrão
  uint32_t tempStep;  // Tamanho do passo do giro
  uint32_t serial;  // Serial
};

// Estado e IPC
class Session {
public:
  Session();

  // Configurações
  struct Settings settings;

  // Estado
  struct State state;

  // Carregar/salvar configurações
  void load();
  void save();
  void reset();

  // Mínimo e máximo aquecimento
  static const float tempMin = 20.0;
  static const float tempMax = 220.0;

  // Sinalizar mudança na sessão
  bool changed;

  // Acionar / parar
  void start();
  void stop();

  // Ativo?
  bool running();

  // Comunicação serial?
  bool serialCom;
};

#endif
