#include "task.h"
#include "monitor.h"
#include "session.h"
#include "display.h"

extern long int encoderMove;

Monitor::Monitor(Session* session, U8X8_SH1106_128X64_NONAME_HW_I2C* display, unsigned long wait): Task(wait), session(session), display(display) {
  
  local = *session;

}

void Monitor::begin(){
  //Serial.begin(115200);
  //Serial.println("Start");

  display->begin();
  display->setFlipMode(1);
  display->setFont(u8x8_font_8x13_1x2_r);
}

void Monitor::action(){
  
  if ( (int)session->temperature != (int)local.temperature ){
    local.temperature = session->temperature;
    session->changed = true;
  }

  if ( encoderMove >= local.encoder + 4 ) {
    local.encoder = encoderMove;
    session->cw();
  }

  if ( encoderMove <= local.encoder - 4 ) {
    local.encoder = encoderMove;
    session->ccw();
  }
  
  if ( session->changed ) show();

}

void Monitor::show(){

  // Posição da linha e conteúdo
  int l;
  String str;

  double gapShow = ( session->on ) ? session->tempeGapTherm : 0;

  l = 0;
  str = "Temp: " + String((int)session->temperature + (int)gapShow);
  printLine(l, str);

  l = 2;
  str = "Alvo: " + String((int)session->tempeTarget + (int)session->tempeGapTherm);
  printLine(l, str);
   
  l = 4;
  str = "Gate: " + String((int)session->PID[7]);
  printLine(l, str);
 
  l = 6;
  if ( session->on ){
    str = "Ligado";
  }
  else {
    str = "Desligado";
 
    // Reset PID
    session->PID[0] = 0;
    session->PID[1] = 0;
    session->PID[2] = 0;
    session->PID[3] = 0;
    session->PID[7] = 0;
  }
  printLine(l, str);

}

void Monitor::printLine(int l, String& str){
  int c = 0;
  display->drawString(c, l, str.c_str());
  if ( str.length() < display->getCols() ){
    c = str.length();
    while ( c < display->getCols() ){
      display->drawString(c, l, " ");
      c++;
    }
  }
}
