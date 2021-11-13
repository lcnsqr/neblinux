#include "task.h"
#include <stdlib.h>

void Task::run(unsigned long time) {
  if ( enabled && time - last >= wait ){
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

void Tasks::run(unsigned long time) {
  for (int t = 0; t != total; ++t) tasks[t]->run(time);
}

