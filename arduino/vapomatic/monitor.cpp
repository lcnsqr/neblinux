#include "task.h"
#include "monitor.h"
#include "session.h"
#include "screen.h"

extern long int encoderMove;

Monitor::Monitor(Session* session, Screen* splash, Screen* screen, int btTop, int btFront, unsigned long wait): Task(wait), session(session), splash(splash), screen(screen), btTop(btTop), btFront(btFront) {
  
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

void Monitor::action(){

  // Avaliar se informações na sessão a serem
  // exibidas mudaram em relação à cópia local.
  // Se sim, atualizar a tela.
  
  // Temperatura atual
  if ( (int)session->tempeCore != (int)local.tempeCore || (int)session->tempeEx != (int)local.tempeEx ){
    local.tempeCore = session->tempeCore;
    session->changed = true;
  }

  // Contador de tempo
  if ( session->elapsed != local.elapsed ){
    local.elapsed = session->elapsed;
    session->changed = true;
  }

  // Detector de encerramento
  if ( session->end[0] != local.end[0] || session->end[1] != local.end[1] ){
    session->changed = true;
  }

  // Avaliar estado de pressionamento dos botões;
  // A ação do botão pode alterar a tela atual.
  btTopSt[0] = btTopSt[1]; // Copiar estado anterior do botão superior
  btTopSt[1] = (digitalRead(btTop) == LOW) ? 1 : 0; // LOW é pressionado
  if ( btTopSt[0] == 0 && btTopSt[1] == 1 ) screen = screen->btTopDown(); // Botão pra baixo
  else if ( btTopSt[0] == 1 && btTopSt[1] == 0 ) screen = screen->btTopUp(); // Botão pra cima
  btFrontSt[0] = btFrontSt[1]; // Copiar estador anterior do botão frontal
  btFrontSt[1] = (digitalRead(btFront) == LOW) ? 1 : 0; // LOW é pressionado
  if ( btFrontSt[0] == 0 && btFrontSt[1] == 1 ) screen = screen->btFrontDown(); // Botão pra baixo
  else if ( btFrontSt[0] == 1 && btFrontSt[1] == 0 ) screen = screen->btFrontUp(); // Botão pra cima

  // Resposta ao rotary também depende da tela atual.
  // O estado está em *encoderMove* no caso do rotary encoder.
  if ( encoderMove >= local.encoder + 4 ) {
    local.encoder = encoderMove;
    screen->cw();
    session->changed = true;
  }
  if ( encoderMove <= local.encoder - 4 ) {
    local.encoder = encoderMove;
    screen->ccw();
    session->changed = true;
  }
  
  if ( session->changed ){

    if ( millis() < 4500 ){
      // Splash screen
      splash->show();
    }
    else {
      // Exibir mudanças na tela ativa
      screen->show();
      // Mudanças exibidas
      session->changed = false;
    }

  }

}

