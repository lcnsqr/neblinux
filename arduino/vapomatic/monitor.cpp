#include "monitor.h"
#include "screen.h"
#include "session.h"
#include "task.h"

extern long int encoderMove;

Monitor::Monitor(Session *session, Screen *screen, int btTop, int btFront,
                 unsigned long wait)
    : Task(wait), session(session), screen(screen), btTop(btTop),
      btFront(btFront) {

  // Cópia local da sessão
  local = *session;

  // Botão superior
  pinMode(btTop, INPUT);
  // turn pullup resistor on
  digitalWrite(btTop, HIGH);
  btTopSt[0] = 0;
  btTopSt[1] = 0;

  // Botão frontal
  pinMode(btFront, INPUT);
  digitalWrite(btFront, HIGH);
  btFrontSt[0] = 0;
  btFrontSt[1] = 0;
}

void Monitor::action() {

  // Avaliar se informações na sessão a serem
  // exibidas mudaram em relação à cópia local.
  // Se sim, atualizar a tela.

  // Temperatura atual
  if ((int)session->tempCore != (int)local.tempCore ||
      (int)session->tempEx != (int)local.tempEx) {
    local.tempCore = session->tempCore;
    session->changed = true;
  }

  // Contador de tempo
  if (session->elapsed != local.elapsed) {
    local.elapsed = session->elapsed;
    session->changed = true;
  }

  // Detector de encerramento
  if (session->shut[0] != local.shut[0] || session->shut[1] != local.shut[1]) {
    session->changed = true;
  }

  // Avaliar estado de pressionamento dos botões;
  // A ação do botão pode alterar a tela atual.
  btTopSt[0] = btTopSt[1]; // Copiar estado anterior do botão superior
  btTopSt[1] = (digitalRead(btTop) == LOW) ? 1 : 0; // LOW é pressionado
  if (btTopSt[0] == 0 && btTopSt[1] == 1)
    screen = screen->btTopDown(); // Botão pra baixo
  else if (btTopSt[0] == 1 && btTopSt[1] == 0)
    screen = screen->btTopUp(); // Botão pra cima
  btFrontSt[0] = btFrontSt[1];  // Copiar estador anterior do botão frontal
  btFrontSt[1] = (digitalRead(btFront) == LOW) ? 1 : 0; // LOW é pressionado
  if (btFrontSt[0] == 0 && btFrontSt[1] == 1)
    screen = screen->btFrontDown(); // Botão pra baixo
  else if (btFrontSt[0] == 1 && btFrontSt[1] == 0)
    screen = screen->btFrontUp(); // Botão pra cima

  // Resposta ao rotary também depende da tela atual.
  // O estado está em *encoderMove* no caso do rotary encoder.
  if (encoderMove >= local.encoder + 4) {
    local.encoder = encoderMove;
    screen->cw();
    session->changed = true;
  }
  if (encoderMove <= local.encoder - 4) {
    local.encoder = encoderMove;
    screen->ccw();
    session->changed = true;
  }

  if (session->changed) {

    // Exibir mudanças na tela ativa
    screen->show();
    // Mudanças exibidas
    session->changed = false;
  }
}
