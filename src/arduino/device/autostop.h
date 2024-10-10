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

  // Mínimo tempo decorrido após última mudança no target
  static const int targetMinSec = 10;

  // Em qual momento aplicar coeficientes integrais
  static const float mark = 120.0;
  // Em qual momento atingir a redução especificada
  static const float over = 240.0;
  // Redução especificada
  static const float decay = log(1.0/3.0);

private:
  Session *session;
};

#endif
