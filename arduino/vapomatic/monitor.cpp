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

  // Contagem
  counting = 0;
  started = 0;
  elapsed = 0;

  // Contagem de tempo para transmissão serial
  serial_before = millis();
}

void Monitor::action() {

  // Avaliar se informações na sessão a serem
  // exibidas mudaram em relação à cópia local.
  // Se sim, atualizar a tela.

  // Comunicação serial
  serial_now = millis();
  if (serial_now - serial_before >= serial_wait) {
    // Enviar estado para o utilitário de setup
    session->state.ts = millis();
    Serial.write((char *)&(session->state), sizeof(struct State));
    serial_before = serial_now;
  }

  if (Serial.available() > 0) {
    // Receber mudanças no estado enviadas pelo utilitário de setup
    Serial.readBytes((char *)&(stateIn), sizeof(struct StateIO));
    // Aguardar recolhimento completo
    delay(100);
    // Modificar estado do aparelho a partir da estrutura enviada
    session->state.tempTarget = stateIn.tempTarget;
    if ( stateIn.on == 1 && session->state.on != true ){
      session->start();
    }
    if ( stateIn.on != 1 && session->state.on == true ){
      session->stop();
    }
    session->state.fan = stateIn.fan;
    session->state.PID[5] = (float)stateIn.PID_enabled;
    if (session->state.PID[5] == 0)
      session->state.PID[4] = stateIn.heat;
    session->changed = true;
  }

  // Temperatura atual
  if ((int)session->state.tempCore != (int)local.state.tempCore ||
      (int)session->state.tempEx != (int)local.state.tempEx) {
    local.state.tempCore = session->state.tempCore;
    session->changed = true;
  }

  // Contador de tempo
  if (session->running() != counting) {
    counting = session->running();
    if (counting) {
      started = millis();
      session->state.elapsed = 0;
      elapsed = 0;
    }
  }
  if (counting) {
    elapsed = millis() - started;
    session->state.elapsed = elapsed / 1000;
  }
  if (session->state.elapsed != local.state.elapsed) {
    local.state.elapsed = session->state.elapsed;
    session->changed = true;
  }

  // Detector de encerramento
  if (session->state.shut[1] != local.state.shut[1]) {
    local.state.shut[1] = session->state.shut[1];
    session->changed = true;
  }

  // Avaliar estado de pressionamento dos botões;
  // A ação do botão pode alterar a tela atual.
  btTopSt[0] = btTopSt[1]; // Copiar estado anterior do botão superior
  btTopSt[1] = (digitalRead(btTop) == LOW) ? 1 : 0; // LOW é pressionado
  if (btTopSt[0] == 1 && btTopSt[1] == 0)
    screen = screen->btTop();  // Botão pra cima
  btFrontSt[0] = btFrontSt[1]; // Copiar estador anterior do botão frontal
  btFrontSt[1] = (digitalRead(btFront) == LOW) ? 1 : 0; // LOW é pressionado
  if (btFrontSt[0] == 1 && btFrontSt[1] == 0)
    screen = screen->btFront(); // Botão pra cima

  // Resposta ao rotary também depende da tela atual.
  // O estado está em *encoderMove* no caso do rotary encoder.
  if (encoderMove >= local.encoder + 4) {
    local.encoder = encoderMove;
    screen->rotate(1);
    session->changed = true;
  }
  if (encoderMove <= local.encoder - 4) {
    local.encoder = encoderMove;
    screen->rotate(0);
    session->changed = true;
  }

  if (session->changed) {

    // Exibir mudanças na tela ativa
    screen->show();
    // Mudanças exibidas
    session->changed = false;
  }
}
