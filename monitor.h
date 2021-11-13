#ifndef Monitor_h
#define Monitor_h

#include "task.h"
#include "session.h"
#include "display.h"

class Monitor: public Task {

  public:

  Monitor(Session* session, Adafruit_SSD1306* display, unsigned long wait);

  void begin();
  void action();
  void show();

  private:

  Session* session;
  Session local;

  Adafruit_SSD1306* display;

};

#endif
