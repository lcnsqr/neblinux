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

  // Contagem de tempo para transmissão serial
  serial_before = millis();

  // Ativar comunicação serial caso botão frontal esteja pressionado ao ligar
  if (digitalRead(btFront) == LOW)
    session->serialCom = true;

  // Screensaver
  standby = 0;
  standby_idle_since = millis();
}

void Monitor::action() {

  // Comunicação serial
  serial_now = millis();
  if (serial_now - serial_before >= serial_wait && session->serialCom) {
    // Enviar estado para o utilitário de setup
    session->state.ts = millis();
    Serial.write((char *)&(session->state), sizeof(struct State));
    serial_before = serial_now;
  }

  if (Serial.available() > 0 && session->serialCom) {

    // Receber mudanças no estado enviadas pelo utilitário de setup
    Serial.readBytes((char *)&(stateIn), sizeof(struct StateIO));

    // Aguardar recolhimento completo
    delay(100);

    // Modificar estado do aparelho a partir da estrutura enviada

    if (session->state.tempTarget != stateIn.tempTarget)
      session->state.targetLastChange = millis() / 1000;

    session->state.tempTarget = stateIn.tempTarget;

    if (stateIn.on == 1 && session->state.on != true) {
      session->start();
    }

    if (stateIn.on != 1 && session->state.on == true) {
      session->stop();
    }

    session->state.fan = stateIn.fan;

    session->state.splash = stateIn.splash;

    session->state.PID_enabled = stateIn.PID_enabled;

    session->state.autostop = stateIn.autostop;

    session->state.tempStep = stateIn.tempStep;

    session->state.serial = stateIn.serial;

    if (session->state.PID_enabled == 0)
      session->state.PID[4] = stateIn.heat;

    // Coeficientes temperatura
    if (stateIn.cTemp[0] != 0) {
      session->state.cTemp[0] = stateIn.cTemp[0];
      session->state.cTemp[1] = stateIn.cTemp[1];
      session->state.cTemp[2] = stateIn.cTemp[2];
      session->state.cTemp[3] = stateIn.cTemp[3];
    }

    // Coeficientes PID
    if (stateIn.cPID[1] != 0) {
      session->state.cPID[0] = stateIn.cPID[0];
      session->state.cPID[1] = stateIn.cPID[1];
      session->state.cPID[2] = stateIn.cPID[2];
    }

    // Limiares de parada
    if (stateIn.cStop[1] != 0) {
      session->state.cStop[0] = stateIn.cStop[0];
      session->state.cStop[1] = stateIn.cStop[1];
    }

    // Resetar definições
    if (stateIn.reset == 1) {
      session->reset();
      stateIn.reset = 0;
    }

    // Gravar definições na EEPROM
    if (stateIn.store == 1) {
      session->save();
      stateIn.store = 0;
    }

    session->changed = true;
  }

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
    if (!standby) {
      screen = screen->btTop(); // Botão pra cima
    }
    session->changed = true;
    standby_idle_since = millis();
    standby = 0;
  }
  btFrontSt[0] = btFrontSt[1]; // Copiar estador anterior do botão frontal
  btFrontSt[1] = (digitalRead(btFront) == LOW) ? 1 : 0; // LOW é pressionado
  if (btFrontSt[0] == 1 && btFrontSt[1] == 0) {
    if (!standby) {
      screen = screen->btFront(); // Botão pra cima
    }
    session->changed = true;
    standby_idle_since = millis();
    standby = 0;
  }

  // Resposta ao rotary também depende da tela atual.
  // O estado está em *encoderMove* no caso do rotary encoder.
  if (encoderMove >= encoderLocal + 4) {
    if (!standby) {
      encoderLocal = encoderMove;
      screen->rotate(1);
    }
    session->changed = true;
    standby_idle_since = millis();
    standby = 0;
  }
  if (encoderMove <= encoderLocal - 4) {
    if (!standby) {
      encoderLocal = encoderMove;
      screen->rotate(0);
    }
    session->changed = true;
    standby_idle_since = millis();
    standby = 0;
  }

  if (millis() > 2000 && millis() < 2100) {
    // Ocultar splash screen
    session->state.splash = 0;
    session->changed = true;
  }

  if (millis() - standby_idle_since > standby_max_idle_time) {
    standby = 1;
    screen->saver();
  }

  if (session->changed && !standby) {
    // Exibir mudanças na tela ativa
    screen->show();
    // Mudanças exibidas
    session->changed = false;
  }
}
