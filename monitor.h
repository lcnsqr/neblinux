#ifndef Monitor_h
#define Monitor_h

#include "task.h"
#include "session.h"
#include "display.h"

class Monitor: public Task {

  public:

  Monitor(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display, unsigned long wait);

  void begin();
  void action();
  void show();
  void splash();

  private:

  Session* session;
  Session local;

  U8G2_SH1106_128X64_NONAME_2_HW_I2C* display;

};

#endif
