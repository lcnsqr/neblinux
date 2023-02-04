#include "log.h"
#include "session.h"
#include "task.h"
#include <Arduino.h>

Log::Log(Session *session, unsigned long wait)
    : Task(wait), session(session) {

}

void Log::action() {
  if ( ! session->running() ) return;

  Serial.print(millis());
  Serial.print("\t");
  Serial.print(session->elapsed);
  Serial.print("\t");

  // Leitura ADC do termistor
  Serial.print(session->analogTherm);
  Serial.print("\t");

  // Temperatura na base da resistência
  Serial.print(session->tempCore);
  Serial.print("\t");

  // Temperatura alvo no exaustor
  Serial.print(session->tempTarget);
  Serial.print("\t");

  // Atuador
  Serial.print((int)session->PID[4]);
  Serial.print("\t");

  // Temperatura no exaustor
  Serial.print(session->tempEx);
  Serial.print("\t");

  // Indicadores de encerramento
  Serial.print(session->tempEx - session->tempTarget);
  Serial.print("\t");
  Serial.print(session->shut[0]);
  Serial.print("\t");
  Serial.print(session->shut[1]);
  Serial.print("\n");
}
