#ifndef Session_h
#define Session_h

class Session {
  public:
  Session();

  bool changed;

  double temperature;
  double tempeTarget;
  double analogTherm;

  // Diferença entre a medição do termistor e a temperatura na saída
  double tempeGapTherm;

  // Rotary push button causes on/off state change
  bool on;

  long int encoder;

  // Rotary encoder clockwise
  void cw();

  // Rotary encoder counter-clockwise
  void ccw();

  // 0,1,2: PID
  // 3: D anterior (atual - alvo)
  // 4,5,6: coeficientes
  // 7: Valor do atuador
  double PID[8];
};

#endif
