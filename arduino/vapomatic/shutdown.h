#ifndef Shutdown_h
#define Shutdown_h

#include "session.h"
#include "task.h"

class Shutdown : public Task {

public:
  Shutdown(Session *session, unsigned long wait);

  void action();

  // Regressão linear em 4 pontos
  static const int pts = 4;
  float x[pts];
  float y[pts];

private:
  Session *session;
};

#endif
