#include "task.h"
#include "session.h"
#include "therm.h"
#include "display.h"
#include "monitor.h"
#include "rotary.h"
#include "fan.h"
#include "heater.h"
#include "timer.h"
#include "shutdown.h"
#include "screen.h"

Session session;
Tasks tasks;
Therm therm(&session, A0, 3*17, 17);
U8G2_SH1106_128X64_NONAME_2_HW_I2C display(U8G2_R2, U8X8_PIN_NONE);

// Telas da UI
scrSplash uiSplash(&session, &display);
scrMain uiMain(&session, &display);
scrDebug uiDebug(&session, &display);

Monitor monitor(&session, &uiSplash, &uiMain, 4, 8, 25);
Rotary rotary;
Fan fan(&session, 7, 75);
Heater heater(&session, 5, 38);
Timer timer(&session, 1000);
Shutdown shutdown(&session, 500);

void setup() {

  // Definições de UI
  uiMain.leave = &uiDebug;
  uiDebug.leave = &uiMain;
  display.begin();


  // Serviços
  tasks.add(&therm);
  tasks.add(&monitor);
  tasks.add(&fan);
  tasks.add(&heater);
  tasks.add(&timer);
  tasks.add(&shutdown);

}

void loop() {
  tasks.run();
}
