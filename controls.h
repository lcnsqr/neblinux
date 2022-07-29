#ifndef Controls_h
#define Controls_h

#include "task.h"
#include "session.h"

class Controls: public Task {

  public:

  Controls(Session* session, int pinA, int pinB, unsigned long wait);

  void action();
  void pushTop();
  void pushFront();

  private:

  // Botão superior
  int pinA;
  unsigned long changedA;
  // Botão frontal
  int pinB;
  unsigned long changedB;
  
  unsigned long repeat;

  Session* session;
};

#endif
