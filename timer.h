#ifndef Timer_h
#define Timer_h

#include "task.h"
#include "session.h"

class Timer: public Task {

  public:

  Timer(Session* session, unsigned long wait);

  void action();

  bool on;

  // Em milissegundos
  unsigned long started;
  unsigned long elapsed;

  private:

  Session* session;

};

#endif

