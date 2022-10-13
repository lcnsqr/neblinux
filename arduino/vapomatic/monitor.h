#ifndef Monitor_h
#define Monitor_h

#include "screen.h"
#include "session.h"
#include "task.h"

class Monitor : public Task {

public:
  Monitor(Session *session, Screen *screen, int btTop, int btFront,
          unsigned long wait);

  void action();

private:
  Session *session;
  Session local;

  Screen *screen;

  // Botão superior
  int btTop;
  int btTopSt[2];

  // Botão frontal
  int btFront;
  int btFrontSt[2];

  // Contagem do tempo ativo em milissegundos
  bool counting;
  unsigned long started;
  unsigned long elapsed;
};

#endif
