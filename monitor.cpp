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
  
  // Temperatura atual
  if ( session->tempeCore != local.tempeCore ){
    local.tempeCore = session->tempeCore;
    session->changed = true;
  }

  // Contador de tempo
  if ( session->elapsed != local.elapsed ){
    local.elapsed = session->elapsed;
    session->changed = true;
  }

  // Meta de temperatura
  if ( encoderMove >= local.encoder + 4 ) {
    local.encoder = encoderMove;
    session->cw();
  }
  if ( encoderMove <= local.encoder - 4 ) {
    local.encoder = encoderMove;
    session->ccw();
  }
  
  if ( session->changed ){

    if ( millis() < 4500 ){
      // Splash screen
      splash();
    }
    else {
      // Exibir mudanças
      show();
      // Mudanças exibidas
      session->changed = false;
    }

  }

}

void Monitor::splash(){

  String str;

  // 9 pixel height
  display->setFont(u8g2_font_7x13B_mf);

  display->firstPage();
  do {
    display->drawBox(56, 2, 6, 31);
    display->drawBox(66, 2, 6, 31);

    if ( millis() > 1500 ){
      str = String("Ganjolito");
      display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 46, str.c_str());
    }
    if ( millis() > 2500 ){
      str = String("Modelo II");
      display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 61, str.c_str());
    }

  } while ( display->nextPage() );
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
    if ( tempeDial < 0 ) tempeDial = 0;
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
    display->setFont(u8g2_font_6x10_mf);

    // Labels
    str = "Atual";
    display->drawUTF8(52 - display->getUTF8Width(str.c_str()), 10, str.c_str());
    str = "Meta";
    display->drawUTF8(76, 10, str.c_str());

    // 9 pixel height
    display->setFont(u8g2_font_7x13B_mf);

    str = String("°C");
    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 45, str.c_str());

    // Status
    str = String(session->elapsed / 60) + "m" + String(session->elapsed % 60) + "s";
    if ( ! session->on ){
      str = (session->elapsed == 0) ? "Parado" : str;
    }

    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 61, str.c_str());

  } while ( display->nextPage() );

}
