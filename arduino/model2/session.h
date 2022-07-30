#ifndef Session_h
#define Session_h

class Session {
  public:
  Session();

  // Tela ativa
  int screen;

  // Sinalizar mudança na sessão
  bool changed;

  // Tempo em segundos em atividade
  unsigned long elapsed;

  // Temperatura na base da resistência
  double tempeCore;
  // Temperatura no exaustor
  double tempeEx;
  // Temperatura alvo no exaustor
  double tempeTarget;
  // Leitura ADC do termistor
  double analogTherm;


  // Indicadores de encerramento
  double end[2];

  // Rotary variável auxiliar
  long int encoder;

  // Rotary encoder clockwise
  void cw();

  // Rotary encoder counter-clockwise
  void ccw();

  // Acionar / parar
  void start();
  void stop();

  // Ativo?
  bool running();

  // 0,1,2: PID
  // 3: D anterior (atual - alvo)
  // 4,5,6: coeficientes
  // 7: Valor do atuador
  double PID[8];

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
