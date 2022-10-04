#ifndef Session_h
#define Session_h

// Estrutura para armazenar na EEPROM
struct Settings {
  double tempCore[3]; // Temperaturas internas na calibragem
  double tempEx[3];   // Temperaturas aferidas na calibragem
  double PID[3];      // Coeficientes PID
  double shutLim[2];  // Limiares de desligamento (y-intercept e slope da função temp - alvo)
	char shutEnabled;   // Desligamento automático ativo/inativo
};

// Estado e IPC
class Session {
  public:
  Session();

  // Carregar configurações
  void load(struct Settings*);

  // Configurações básicas
  struct Settings* settings;

  // Sinalizar mudança na sessão
  bool changed;

  // Tempo em segundos em atividade
  int elapsed;

  // Temperatura na base da resistência
  double tempeCore;
  // Temperatura no exaustor
  double tempeEx;
  // Temperatura alvo no exaustor
  double tempeTarget;
  // Leitura ADC do termistor
  double analogTherm;


  // Coeficientes do polinômio de segunda
  // ordem que estima a temperatura na saída
  // thCfs[0] : Coeficientes usados quando desativado
  // thCfs[1] : Coeficientes usados quando ativado
  double thCfs[2][3];

  // Calcular coeficiente do polinômio interpolador de temperatura
  void leastsquares(int m, int n, double x[], double y[], double c[]);

  // Indicadores de encerramento
  double shut[2];

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
  double PID[5];

  // Modo teste, não aciona resistência e fan
  bool dryrun;

  // Temperatura permitida
  double tempeMin;
  double tempeMax;

  private:

  // On/off state change
  bool on;

};

#endif
