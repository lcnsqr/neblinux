#ifndef Fan_h
#define Fan_h

#include "task.h"
#include "session.h"

class Fan: public Task {

  public:

  Fan(Session* session, int pin, unsigned long wait);

  void action();

  private:

  // Relay pin
  int pin;

  Session* session;

};

#endif
