#include <Arduino.h>
#include "task.h"
#include "session.h"
#include "therm.h"
#include "controls.h"
#include "display.h"
#include "monitor.h"
#include "rotary.h"
#include "sampler.h"
#include "fan.h"
#include "heater.h"
#include "timer.h"

Session session;
Tasks tasks;
Therm therm(&session, A0, 3*17, 17);
Controls controls(&session, 4, 40);
U8G2_SH1106_128X64_NONAME_2_HW_I2C display(U8G2_R2, U8X8_PIN_NONE);
Monitor monitor(&session, &display, 25);
Rotary rotary;
Sampler sampler(&therm, A0, 500);
Fan fan(&session, 7, 75);
Heater heater(&session, 5, 75);
Timer timer(&session, 1000);

void setup() {
  monitor.begin();

  tasks.add(&therm);
  tasks.add(&controls);
  tasks.add(&monitor);
  tasks.add(&sampler);
  tasks.add(&fan);
  tasks.add(&heater);
  tasks.add(&timer);

}

void loop() {
  tasks.run();
}
