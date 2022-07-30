#ifndef Shutdown_h
#define Shutdown_h

#include "task.h"
#include "session.h"

class Shutdown: public Task {

  public:

  Shutdown(Session* session, unsigned long wait);

  void action();

  // Regressão linear em 4 pontos
  double b[4];

  // Matriz A'A da regressão linear
  double AA[4];

  // Vetor A'b da regressão linear
  double Ab[2];

  // Coeficientes encontrados (y-intercept e slope)
  double c[2];

  // Limiares de desligamento
  double lim[2];

  private:

  Session* session;

};

#endif

