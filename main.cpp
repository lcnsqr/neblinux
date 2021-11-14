#include "task.h"
#include "session.h"
#include "therm.h"
#include "controls.h"
#include "display.h"
#include "monitor.h"
#include <Arduino.h>
#include "rotary.h"

Session session;
Tasks tasks;
Therm therm(&session, A0, 169, 13);
Controls controls(&session, 4, 40);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Monitor monitor(&session, &display, 25);
Rotary rotary;

void setup() {
  monitor.begin();

  tasks.add(&therm);
  tasks.add(&controls);
  tasks.add(&monitor);

}

void loop() {
  tasks.run(millis());
}
