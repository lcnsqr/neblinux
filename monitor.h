#ifndef Monitor_h
#define Monitor_h

#include "task.h"
#include "session.h"
#include "display.h"

class Monitor: public Task {

  public:

  Monitor(Session* session, U8X8_SH1106_128X64_NONAME_HW_I2C* display, unsigned long wait);

  void begin();
  void action();
  void show();
  void printLine(int, String&);

  private:

  Session* session;
  Session local;

  U8X8_SH1106_128X64_NONAME_HW_I2C* display;

};

#endif
