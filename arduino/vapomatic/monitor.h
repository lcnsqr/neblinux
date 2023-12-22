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

  // Intervalo de transmissão serial
  static const float serial_wait = 250;
  long int serial_before, serial_now;

  // Buffer de recepção do estado
  struct StateIO stateIn;

private:
  Session *session;

  // Intervalo do protetor de tela
  static const int32_t screensaver_max_idle_time = 300000;
  int32_t screensaver_idle_since;
  uint8_t screensaver;

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
