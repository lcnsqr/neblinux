#ifndef Shutdown_h
#define Shutdown_h

#include "session.h"
#include "task.h"

class Shutdown : public Task {

public:
  Shutdown(Session *session, unsigned long wait);

  void action();

  // Regressão linear em 4 pontos
  float b[4];

  // Matriz A'A da regressão linear
  float AA[4];

  // Vetor A'b da regressão linear
  float Ab[2];

private:
  Session *session;
};

#endif
