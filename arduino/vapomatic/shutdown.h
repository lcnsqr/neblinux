#ifndef Shutdown_h
#define Shutdown_h

#include "session.h"
#include "task.h"

class Shutdown : public Task {

public:
  Shutdown(Session *session, unsigned long wait);

  void action();

  static const int pts = 3;
  float x[pts];
  float y[pts];

  // Mínimo tempo decorrido
  static const int minsec = 60;
  // Declive negativo importante na carga indica enchimento
  static const float slope = -10.0;

private:
  Session *session;
};

#endif
