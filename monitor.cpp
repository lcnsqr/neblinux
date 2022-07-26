#include "task.h"
#include "monitor.h"
#include "session.h"
#include "display.h"

extern long int encoderMove;

Monitor::Monitor(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display, unsigned long wait): Task(wait), session(session), display(display) {
  
  local = *session;

}

void Monitor::begin(){
  //Serial.begin(115200);
  //Serial.println("Start");

  display->begin();
}

void Monitor::action(){
  
  if ( (int)session->tempeCore != (int)local.tempeCore ){
    local.tempeCore = session->tempeCore;
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

  // String para exibir labels e valores
  String str;

  // Valor na escala de temperaturas
  u8g2_uint_t tempeDial;

  display->firstPage();
  do {

    // Escala leitura
    display->drawFrame(56, 2, 6, 31);
    tempeDial = (u8g2_uint_t)round(31.0 * (session->tempeEx - session->tempeMin) / (session->tempeMax - session->tempeMin));
    display->drawBox(56, 33 - tempeDial, 6, tempeDial);

    // Escala objetivo
    display->drawFrame(66, 2, 6, 31);
    tempeDial = (u8g2_uint_t)round(31.0 * (session->tempeTarget - session->tempeMin) / (session->tempeMax - session->tempeMin));
    display->drawBox(66, 33 - tempeDial, 6, tempeDial);

    // 16 pixel height
    display->setFont(u8g2_font_inb16_mn);

    // Valores leitura e objetivo
    str = String((int)session->tempeEx);
    display->drawStr(52 - display->getStrWidth(str.c_str()), 33, str.c_str());
    str = String((int)session->tempeTarget);
    display->drawStr(76, 33, str.c_str());

    // 7 pixel height
    display->setFont(u8g2_font_6x10_mr);

    // Labels
    str = "Leitura";
    display->drawStr(52 - display->getStrWidth(str.c_str()), 10, str.c_str());
    str = "Objetivo";
    display->drawStr(76, 10, str.c_str());

    // 9 pixel height
    display->setFont(u8g2_font_7x13B_mf);

    str = String("°C");
    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 45, str.c_str());

    // Status
    if ( session->on ){
      str = "Ativo";
    }
    else {
      str = "Inativo";
    }

    // 10 pixel height
    display->setFont(u8g2_font_9x18B_mr);

    display->drawStr((int)(round((double)(128 - display->getStrWidth(str.c_str()))/2.0)), 63, str.c_str());

  } while ( display->nextPage() );

  /*
  // Posição da linha e conteúdo
  int l;
  String str;

  l = 0;
  str = String((int)session->analogTherm) + " " + String((int)session->tempeCore) + " " + String((int)session->tempeEx);
  printLine(l, str);

  l = 2;
  str = "Alvo: " + String((int)session->tempeTarget);
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
  */

}

void Monitor::printLine(int l, String& str){
  /*
  int c = 0;
  display->drawString(c, l, str.c_str());
  if ( str.length() < display->getCols() ){
    c = str.length();
    while ( c < display->getCols() ){
      display->drawString(c, l, " ");
      c++;
    }
  }
  */
}
