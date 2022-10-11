#ifndef Timer_h
#define Timer_h

#include "session.h"
#include "task.h"

class Timer : public Task {

public:
  Timer(Session *session, unsigned long wait);

  void action();

  bool on;

  // Em milissegundos
  unsigned long started;
  unsigned long elapsed;

private:
  Session *session;
};

#endif
