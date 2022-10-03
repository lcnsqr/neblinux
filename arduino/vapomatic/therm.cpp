#include "task.h"
#include "therm.h"
#include <math.h>
#include <Arduino.h>
#include <stdlib.h>

Therm::Therm(Session* session, int pin, unsigned long wait, unsigned int bufLen): Task(wait), session(session), pin(pin), bufLen(bufLen) {
  r1 = 95000.0;

  thermNominal = 100000.0;
  bCoef = 3950.0;
  tempNominal =  25.0;

  // Dividir o tempo de espera para
  // distribuir as amostras no intervalo
  wait = wait / bufLen;

  buf = (int*) malloc(bufLen*sizeof(int));

  bufCount = 0;
  while (bufCount < bufLen)
    buf[bufCount++] = analogRead(pin);

}

Therm::~Therm(){
  if ( bufLen > 0 ) free(buf);
}

void Therm::action(){
  int bufSum = 0;
  if ( bufCount == bufLen ){
    bufCount = 0;
    for (int i = 0; i < bufLen; ++i)
      bufSum += buf[i];
    session->analogTherm = (double)bufSum/(double)bufLen;
    session->tempeCore = celsiusSteinhart(session->analogTherm);
    session->tempeEx = celsiusPoly(session->tempeCore);
  }
  else {
    buf[bufCount++] = analogRead(pin);
  }
}

double Therm::celsiusPoly(double core){
  const int i = (int)session->running();
  return session->thCfs[i][0] * pow(core, 2) + session->thCfs[i][1] * core + session->thCfs[i][2];
}

double Therm::celsiusSteinhart(double thermistor){
  double r0 = r1 / ((1023.0 / thermistor) - 1.0);

  double steinhart;
  steinhart = r0 / thermNominal;
  steinhart = log(steinhart);
  steinhart /= bCoef;
  steinhart += 1.0 / (tempNominal + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  return steinhart;
}
