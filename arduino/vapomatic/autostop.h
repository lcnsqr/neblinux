#ifndef Autostop_h
#define Autostop_h

#include "session.h"
#include "task.h"

class Autostop : public Task {

public:
  Autostop(Session *session, unsigned long wait);

  void action();

  static const int pts = 4;

  // Elementos da matriz A' * A, onde A = [1 0; 1 0.25; 1 0.5; 1 0.75]
  static const float a0 = 4.0;
  static const float a1 = 1.5;
  static const float a2 = 1.5;
  static const float a3 = 0.875;

  // Domínio normalizado em [0,1)
  static const float x0 = 0;
  static const float x1 = 0.25;
  static const float x2 = 0.5;
  static const float x3 = 0.75;

  // Última leituras de tempEx e carga
  float tempEx[pts];
  float heat[pts];

  // Índice inicial dinâmico
  int iy;

  // RHS
  float s[2];

  // Mínimo tempo decorrido
  static const int minSec = 60;

  // Constante de decaimento dos limiares de parada
  float decay;

  // Mínimo tempo decorrido após última mudança no target
  static const int targetMinSec = 10;

private:
  Session *session;
};

#endif
