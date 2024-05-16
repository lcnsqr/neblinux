#include "task.h"
#include <Arduino.h>
#include <stdlib.h>

void Task::run(unsigned long time) {
  if (enabled && ((last < time) ? time - last : ~0 - last + time) >= wait) {
    action();
    last = time;
  }
}

void Tasks::run() {
  for (int t = 0; t != total; ++t)
    tasks[t]->run(millis());
}
