#include "task.h"
#include "sampler.h"
#include <Arduino.h>

Sampler::Sampler(Therm* th, int pin, unsigned long wait): Task(wait), th(th), pin(pin) {

}

void Sampler::action(){
  int input = analogRead(pin);
  //Serial.print(input);
  //Serial.print("	");
  //Serial.println(th->celsius((double)input));
}
