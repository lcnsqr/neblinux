#include "screen.h"
#include "session.h"
#include "display.h"

Screen::Screen(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): session(session), display(display) {
  
}

/***
 * Tela de apresentação
 */
scrSplash::scrSplash(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): Screen(session, display) { }

void scrSplash::show(){

  String str;

  display->firstPage();
  do {
    // 10 pixel height
    display->setFont(u8g2_font_8x13B_mf);

    str = String("VAPOMATIC");
    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 32, str.c_str());

    // 7 pixel height
    display->setFont(u8g2_font_6x10_mf);
    
    str = String("vapomatic.com.br");
    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 46, str.c_str());

  } while ( display->nextPage() );
}

void scrSplash::cw(){ }
void scrSplash::ccw(){ }
Screen* scrSplash::btTopDown(){return this;}
Screen* scrSplash::btTopUp(){return this;}
Screen* scrSplash::btFrontDown(){return this;}
Screen* scrSplash::btFrontUp(){return this;}

/***
 * Tela principal
 */
scrMain::scrMain(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): Screen(session, display) {
  leave = NULL;
}

void scrMain::show(){
  // String para exibir labels e valores
  String str;

  // Valor na escala de temperaturas
  u8g2_uint_t tempeDial;

  // Desconsiderar temperaturas fora da faixa para a escala visual
  double tempeEx;

  display->firstPage();
  do {

    // Escala leitura
    display->drawFrame(56, 2, 6, 31);
    tempeEx = session->tempeEx;
    if ( tempeEx < session->tempeMin ) tempeEx = session->tempeMin;
    if ( tempeEx > session->tempeMax ) tempeEx = session->tempeMax;
    tempeDial = round(31.0 * (tempeEx - session->tempeMin) / (session->tempeMax - session->tempeMin));
    display->drawBox(56, (u8g2_uint_t)(33 - tempeDial), 6, (u8g2_uint_t)tempeDial);

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
    str = "Temp";
    display->drawUTF8(52 - display->getUTF8Width(str.c_str()), 10, str.c_str());
    str = "Alvo";
    display->drawUTF8(76, 10, str.c_str());

    // 10 pixel height
    display->setFont(u8g2_font_8x13B_mf);

    str = String("°C");
    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 45, str.c_str());

    // Status
    str = String(session->elapsed / 60) + "m" + String(session->elapsed % 60) + "s";
    if ( ! session->running() ){
      str = (session->elapsed == 0) ? "Parado" : str;
    }

    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 61, str.c_str());

  } while ( display->nextPage() );

}

void scrMain::cw(){
  if ( session->tempeTarget + 10 > session->tempeMax ) return;
  session->tempeTarget += 10;
}

void scrMain::ccw(){
  if ( session->tempeTarget - 10 < session->tempeMin ) return;
  session->tempeTarget -= 10;
}

Screen* scrMain::btTopDown(){return this;}

Screen* scrMain::btTopUp(){
  if ( ! session->running() ){
    session->start();
  }
  else {
    session->stop();
  }
  return this;
}

Screen* scrMain::btFrontDown(){return this;}

Screen* scrMain::btFrontUp(){
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}

/***
 * Tela debug
 */
scrDebug::scrDebug(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): Screen(session, display) {
  leave = NULL;
}

void scrDebug::show(){

  String str;

  u8g2_uint_t y;

  // 7 pixel height
  display->setFont(u8g2_font_6x10_mf);

  display->firstPage();
  do {

    // Valor do thermistor
    y = 10;
    str = "Therm:";
    display->drawUTF8(0, y, str.c_str());
    str = String((int)session->analogTherm);
    display->drawUTF8(62 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // Temperatura na base da resistência
    y = 20;
    str = "Core:";
    display->drawUTF8(0, y, str.c_str());
    str = String((int)session->tempeCore);
    display->drawUTF8(62 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // Temperatura estimada no exaustor ativo
    y = 30;
    str = "Ex:";
    display->drawUTF8(0, y, str.c_str());
    str = String((int)session->tempeEx);
    display->drawUTF8(62 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // Temperatura alvo no exaustor
    y = 40;
    str = "Alvo:";
    display->drawUTF8(0, y, str.c_str());
    str = String((int)session->tempeTarget);
    display->drawUTF8(62 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // Tempo ativo
    y = 50;
    str = "Tempo:";
    display->drawUTF8(0, y, str.c_str());
    str = String(session->elapsed);
    display->drawUTF8(62 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // Carga na resistência
    y = 10;
    str = "Gate:";
    display->drawUTF8(72, y, str.c_str());
    str = String((int)session->PID[7]);
    display->drawUTF8(128 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // PID
    y = 20;
    str = "P:";
    display->drawUTF8(72, y, str.c_str());
    str = String(session->PID[0]);
    display->drawUTF8(128 - display->getUTF8Width(str.c_str()), y, str.c_str());

    y = 30;
    str = "I:";
    display->drawUTF8(72, y, str.c_str());
    str = String(session->PID[1]);
    display->drawUTF8(128 - display->getUTF8Width(str.c_str()), y, str.c_str());

    y = 40;
    str = "D:";
    display->drawUTF8(72, y, str.c_str());
    str = String(session->PID[2]);
    display->drawUTF8(128 - display->getUTF8Width(str.c_str()), y, str.c_str());

    // Indicadores de encerramento
    y = 64;
    str = String(session->end[0]) + "   " + String(session->end[1]);
    display->drawUTF8(128 - display->getUTF8Width(str.c_str()), y, str.c_str());

  } while ( display->nextPage() );

}

void scrDebug::cw(){ }
void scrDebug::ccw(){ }

Screen* scrDebug::btTopDown(){return this;}

Screen* scrDebug::btTopUp(){return this;}

Screen* scrDebug::btFrontDown(){return this;}

Screen* scrDebug::btFrontUp(){
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}
