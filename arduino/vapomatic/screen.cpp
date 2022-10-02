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

  // Valor formatado
  String strVal;

  // 7 pixel height
  display->setFont(u8g2_font_6x10_mf);

  display->firstPage();
  do {

    for(int i = 0; i < nitems; ++i){
      display->drawUTF8(( i < 6 ) ? 0 : 72, ((i%6)+1)*10, items[i].label.c_str());
      if ( items[i].sessionType == INT ){
        strVal = String(*(items[i].value.i));
      }
      else { // Double
        if ( items[i].screenType == INT ){
          strVal = String((int)*(items[i].value.d));
        }
        else {
          strVal = String(*(items[i].value.d));
        }
      }
      display->drawUTF8(62 + (( i < 6 ) ? 0 : 66) - display->getUTF8Width(strVal.c_str()), ((i%6)+1)*10, strVal.c_str());
    }

  } while ( display->nextPage() );

}

void scrDebug::cw(){
  if ( session->tempeTarget + 10 > session->tempeMax ) return;
  session->tempeTarget += 10;
}

void scrDebug::ccw(){
  if ( session->tempeTarget - 10 < session->tempeMin ) return;
  session->tempeTarget -= 10;
}

Screen* scrDebug::btTopDown(){return this;}

Screen* scrDebug::btTopUp(){
  if ( ! session->running() ){
    session->start();
  }
  else {
    session->stop();
  }
  return this;
}

Screen* scrDebug::btFrontDown(){return this;}

Screen* scrDebug::btFrontUp(){
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}
