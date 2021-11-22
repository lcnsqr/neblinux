#include "task.h"
#include "session.h"
#include "therm.h"
#include "controls.h"
#include "display.h"
#include "monitor.h"
#include <Arduino.h>
#include "rotary.h"
#include "sampler.h"

Session session;
Tasks tasks;
Therm therm(&session, A0, 169, 1);
Controls controls(&session, 4, 40);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Monitor monitor(&session, &display, 25);
Rotary rotary;
Sampler sampler(&therm, A0, 500);

void setup() {
  monitor.begin();

  tasks.add(&therm);
  tasks.add(&controls);
  tasks.add(&monitor);
  tasks.add(&sampler);

}

void loop() {
  tasks.run(millis());
}
