#include "task.h"
#include "shutdown.h"
#include "session.h"
#include <Arduino.h>

Shutdown::Shutdown(Session* session, unsigned long wait): Task(wait), session(session) {
  
  b[0] = 0;
  b[1] = 0;
  b[2] = 0;
  b[3] = 0;

  AA[0] = 4.0;
  AA[1] = 6.0;
  AA[2] = 6.0;
  AA[3] = 14.0;

  Ab[0] = 0;
  Ab[1] = 0;

  c[0] = 0;
  c[1] = 0;
}

void Shutdown::action(){
  
  if ( ! session->running() ) return;

  b[0] = b[1];
  b[1] = b[2];
  b[2] = b[3];
  b[3] = session->tempeEx - session->tempeTarget;

  Ab[0] = b[0] + b[1] + b[2] + b[3];
  Ab[1] = b[1] + 2.0*b[2] + 3.0*b[3];

  c[1] = (Ab[1]/AA[2]-Ab[0]/AA[0]) / (AA[3]/AA[2]-AA[1]/AA[0]);
  c[0] = Ab[0]/AA[0] - c[1] * AA[1]/AA[0];

  session->end[0] = c[0];
  session->end[1] = c[1];

  // Desligar se detectado crescimento
  // íngreme da distância temp - alvo.
  // Se y-intercept positivo e slope superior a 5
  if ( c[0] > 0 && c[1] > 5.0 ){
    session->stop();
  }
}

