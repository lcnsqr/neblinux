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

  // Buffer de recepção do estado
  struct StateIO stateIn;

private:
  Session *session;

  // Cópia local do tempo em segundos em atividade
  int32_t elapsedLocal;

  // Cópia local da temperatura no exaustor
  float tempEx;

  // Cópia local do movimento do rotary
  long int encoderLocal;

  // Tela ativa
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
