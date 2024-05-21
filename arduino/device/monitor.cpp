#include "monitor.h"
#include "screen.h"
#include "session.h"
#include "task.h"

extern long int encoderMove;

Monitor::Monitor(Session *session, Screen *screen, int btTop, int btFront,
                 unsigned long wait)
    : Task(wait), session(session), screen(screen), btTop(btTop),
      btFront(btFront) {

  // Cópias locais se variáveis de sessão
  tempEx = session->state.tempEx;
  elapsed = session->state.elapsed;

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

  // Screensaver
  screensaver = 0;
  screensaver_idle_since = millis();
}

void Monitor::action() {

  // Receber comando via porta serial
  if ( session->serialIn == SERIAL_NONE ) {
    if (Serial.available() > 0 ){
      Serial.readBytes((char *)&(session->serialIn), 1);
      screensaver_idle_since = millis();
      screensaver = 0;
    }
  }

  if ( session->serialIn == SERIAL_READ ) {

    // Enviar estado segmentado para não estourar o buffer de saída
    session->state.ts = millis();
    for (int b = 0; b < sizeof(struct State); b += 4){
      Serial.write((char*)((char*)&(session->state) + b), 4);
    }

  } else if ( session->serialIn == SERIAL_WRITE ) {

    // Recebimento de estado
    Serial.readBytes((char *)&stateIn, sizeof(struct StateIO));

    // Verificar integridade do recebido
    if ( stateIn.serialCheck == SERIAL_TAG ){

      // Modificar estado do aparelho a partir da estrutura enviada

      if (session->state.tempTarget != stateIn.tempTarget)
        session->state.targetLastChange = millis() / 1000;

      session->state.tempTarget = stateIn.tempTarget;
      session->state.fan = stateIn.fan;
      session->state.splash = stateIn.splash;
      session->state.PID_enabled = stateIn.PID_enabled;
      session->state.autostop = stateIn.autostop;
      session->state.tempStep = stateIn.tempStep;
      session->state.screensaver = stateIn.screensaver;

      if (session->state.PID_enabled == 0)
        session->state.PID[4] = stateIn.heat;

      // Coeficientes temperatura
      session->state.cTemp[0] = stateIn.cTemp[0];
      session->state.cTemp[1] = stateIn.cTemp[1];
      session->state.cTemp[2] = stateIn.cTemp[2];
      session->state.cTemp[3] = stateIn.cTemp[3];

      // Coeficientes PID
      session->state.cPID[0] = stateIn.cPID[0];
      session->state.cPID[1] = stateIn.cPID[1];
      session->state.cPID[2] = stateIn.cPID[2];

      // Limiares de parada
      session->state.cStop[0] = stateIn.cStop[0];
      session->state.cStop[1] = stateIn.cStop[1];

      session->changed = true;
    }

  } else if ( session->serialIn == SERIAL_START ) {
    session->start();
  } else if ( session->serialIn == SERIAL_STOP ) {
    session->stop();
  } else if ( session->serialIn == SERIAL_RESET ) {
    session->reset();
  } else if ( session->serialIn == SERIAL_STORE ) {
    session->save();
  }

  // Liberar nova chegada de comando
  session->serialIn = SERIAL_NONE;

  // Avaliar se informações na sessão a serem
  // exibidas mudaram em relação à cópia local.
  // Se sim, atualizar a tela.

  // Temperatura atual
  if ((int)session->state.tempEx != (int)tempEx) {
    tempEx = session->state.tempEx;
    session->changed = true;
  }

  // Contador de tempo
  if (session->running() != counting) {
    counting = session->running();
    if (counting) {
      started = millis();
      session->state.elapsed = 0;
      elapsedLocal = 0;
    }
  }
  if (counting) {
    elapsed = millis() - started;
    session->state.elapsed = elapsed / 1000;
  }
  if (session->state.elapsed != elapsedLocal) {
    elapsedLocal = session->state.elapsed;
    session->changed = true;
  }

  // Avaliar estado de pressionamento dos botões;
  // A ação do botão pode alterar a tela atual.
  btTopSt[0] = btTopSt[1]; // Copiar estado anterior do botão superior
  btTopSt[1] = (digitalRead(btTop) == LOW) ? 1 : 0; // LOW é pressionado
  if (btTopSt[0] == 1 && btTopSt[1] == 0) {
    if (!screensaver) {
      screen = screen->btTop(); // Botão pra cima
    }
    session->changed = true;
    screensaver_idle_since = millis();
    screensaver = 0;
  }
  btFrontSt[0] = btFrontSt[1]; // Copiar estador anterior do botão frontal
  btFrontSt[1] = (digitalRead(btFront) == LOW) ? 1 : 0; // LOW é pressionado
  if (btFrontSt[0] == 1 && btFrontSt[1] == 0) {
    if (!screensaver) {
      screen = screen->btFront(); // Botão pra cima
    }
    session->changed = true;
    screensaver_idle_since = millis();
    screensaver = 0;
  }

  // Resposta ao rotary também depende da tela atual.
  // O estado está em *encoderMove* no caso do rotary encoder.
  if (encoderMove >= encoderLocal + 4) {
    if (!screensaver) {
      encoderLocal = encoderMove;
      screen->rotate(1);
    }
    session->changed = true;
    screensaver_idle_since = millis();
    screensaver = 0;
  }
  if (encoderMove <= encoderLocal - 4) {
    if (!screensaver) {
      encoderLocal = encoderMove;
      screen->rotate(0);
    }
    session->changed = true;
    screensaver_idle_since = millis();
    screensaver = 0;
  }

  if (millis() > 2000 && millis() < 2100) {
    // Ocultar splash screen
    session->state.splash = 0;
    session->changed = true;
  }

  if (millis() - screensaver_idle_since > screensaver_max_idle_time && session->state.screensaver) {
    screensaver = 1;
    screen->saver();
  }

  if (session->changed && !screensaver) {
    // Exibir mudanças na tela ativa
    screen->show();
    // Mudanças exibidas
    session->changed = false;
  }
}
