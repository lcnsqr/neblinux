#ifndef Session_h
#define Session_h

#include "state.h"

// Estrutura para armazenar na EEPROM
struct Settings {
  float cTemp[4];    // Coeficientes do polinômio grau 3 que infere temperatura
  float cPID[3];     // Coeficientes PID
  bool autostop;     // Desligamento automático ativo/inativo
  float cStop[2];    // Limiares de parada dos coeficientes de reta para
                     // temperatura e carga
  float tempTarget;  // Temperatura alvo padrão
  uint32_t tempStep; // Tamanho do passo do giro
  uint32_t screensaver;  // Usar descanso de tela ou não
  uint32_t fan;      // Velocidade do fan (100-255)
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
  static const float tempMax = 210.0;

  // Sinalizar mudança na sessão
  bool changed;

  // Acionar / parar
  void start();
  void stop();

  // Ativo?
  bool running();

  // Requisição via porta serial
  // 0x00 → Nada
  // 0x10 → Requisição de estado da sessão
  // 0x11 → Recebimento de estado da sessão
  uint8_t serialIn;

  // Intervalo do protetor de tela
  static const int32_t screensaver_max_idle_time = 60000;
  int32_t screensaver_idle_since;
  uint8_t screensaver;

  // Estado da proteção de tela
  struct {
    // Ponto leve e ponto pesado
    float p[2][2];
    // Direção do ponto
    float d[2][2];
    // Maior deslocamento possível num passo
    static const float L = 5.0;
  } ss;
};

#endif
