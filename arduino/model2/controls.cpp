#include "task.h"
#include "controls.h"
#include <Arduino.h>

Controls::Controls(Session* session, int pinA, int pinB, unsigned long wait): Task(wait), session(session), pinA(pinA), pinB(pinB) {

  // Botão superior
  pinMode(pinA, INPUT);
  // turn pullup resistor on
  digitalWrite(pinA, HIGH);
  changedA = 0;

  // Botão frontal
  pinMode(pinB, INPUT);
  digitalWrite(pinB, HIGH);
  changedB = 0;

  repeat = 2000;

}

void Controls::pushTop(){

  // Botão Superior
  if(digitalRead(pinA) == LOW && ! session->running()){

    // Evitar mudança imediata de estado
    if ( millis() - changedA > repeat ) changedA = millis();
    else return;

    session->start();
  }

  if(digitalRead(pinA) == LOW && session->running()){

    // Evitar mudança imediata de estado
    if ( millis() - changedA > repeat ) changedA = millis();
    else return;

    session->stop();
  }

}

void Controls::pushFront(){

  // Botão Frontal
  if(digitalRead(pinB) == LOW && session->screen == 0){

    // Evitar mudança imediata de estado
    if ( millis() - changedB > repeat ) changedB = millis();
    else return;

    // Alternar tela
    session->screen = 1;
    session->changed = true;
  }

  if(digitalRead(pinB) == LOW && session->screen == 1){

    // Evitar mudança imediata de estado
    if ( millis() - changedB > repeat ) changedB = millis();
    else return;

    // Alternar tela
    session->screen = 0;
    session->changed = true;
  }

}

void Controls::action(){
  pushTop();
  pushFront();
}
