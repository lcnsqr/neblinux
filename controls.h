#ifndef Controls_h
#define Controls_h

#include "task.h"
#include "session.h"

class Controls: public Task {

  public:

  Controls(Session* session, int pin, unsigned long wait);

  void action();

  private:

  // Digital in (push button)
  int pin;
  
  unsigned long changed;
  unsigned long repeat;

  Session* session;
};

#endif
