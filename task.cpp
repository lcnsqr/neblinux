#include "task.h"
#include <stdlib.h>
#include <Arduino.h>

void Task::run(unsigned long time) {
  if ( enabled && (( last < time ) ? time - last : ~ 0 - last + time) >= wait ){
    action();
    last = time;
  }
}

Tasks::~Tasks() {
  if ( total > 0 ) free(tasks);
}

void Tasks::add(Task* task) {
  tasks = (Task**)realloc(tasks, ++total*sizeof(Task**));
  tasks[total-1] = task;
}

void Tasks::run() {
  for (int t = 0; t != total; ++t) tasks[t]->run(millis());
}

