#ifndef Monitor_h
#define Monitor_h

#include "task.h"
#include "session.h"
#include "screen.h"

class Monitor: public Task {

  public:

  Monitor(Session* session, Screen* splash, Screen* screen, int btTop, int btFront, unsigned long wait);

  void action();

  private:

  Session* session;
  Session local;

  Screen* splash;
  Screen* screen;

  // Botão superior
  int btTop;
  int btTopSt[2];

  // Botão frontal
  int btFront;
  int btFrontSt[2];
  
};

#endif
