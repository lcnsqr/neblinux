#ifndef Fan_h
#define Fan_h

#include "task.h"
#include "session.h"

class Fan: public Task {

  public:

  Fan(int port, Session* session, unsigned long wait);

  void action();

  private:
  int port;
  Session* session;

};

#endif
