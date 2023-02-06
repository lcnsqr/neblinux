#ifndef Session_h
#define Session_h

// Estrutura para armazenar na EEPROM
struct Settings {
  float tempCore[3]; // Temperaturas internas na calibragem
  float tempEx[3];   // Temperaturas aferidas na calibragem
  float PID[3];      // Coeficientes PID
  bool shutEnabled; // Desligamento automático ativo/inativo
  float tempTarget; // Temperatura alvo padrão
};

// Estado e IPC
class Session {
public:
  Session();

  // Carregar/salvar configurações
  void load();
  void save();
  void reset();

  // Mínimo e máximo aquecimento
  static const float tempMin = 20.0;
  static const float tempMax = 220.0;

  // Configurações
  struct Settings settings;

  // Sinalizar mudança na sessão
  bool changed;

  // Tempo em segundos em atividade
  int elapsed;

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

  // Rotary variável auxiliar
  long int encoder;

  // Acionar / parar
  void start();
  void stop();

  // Ativo?
  bool running();

  // 0,1,2: PID
  // 3: D anterior (atual - alvo)
  // 4: Valor do atuador
  float PID[5];

private:
  // On/off state change
  bool on;
};

#endif
