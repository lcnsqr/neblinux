#include "screen.h"
#include "session.h"
#include "display.h"
#include "mat.h"


Screen::Screen(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): session(session), display(display) {
  
}

/***
 * Tela principal
 */
scrMain::scrMain(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): Screen(session, display) {
  leave = NULL;
}

void scrMain::splash(){

  String str;

  // 9 pixel height
  display->setFont(u8g2_font_6x13_mf);
  display->setDrawColor(1);

  display->firstPage();
  do {

    str = String("VAPOMATIC");
    display->drawUTF8((int)(round((float)(128 - display->getUTF8Width(str.c_str()))/2.0)), 32, str.c_str());

    str = String("vapomatic.com.br");
    display->drawUTF8((int)(round((float)(128 - display->getUTF8Width(str.c_str()))/2.0)), 46, str.c_str());

  } while ( display->nextPage() );
}

void scrMain::show(){

  if ( millis() < 4500 ){
    splash();
    return;
  }

  // String para exibir labels e valores
  String str;

  // Valor na escala de temperaturas
  u8g2_uint_t tempeDial;

  // Desconsiderar temperaturas fora da faixa para a escala visual
  float tempEx;

  display->firstPage();
  do {

    // 9 pixel height
    display->setFont(u8g2_font_6x13_mf);
    display->setDrawColor(1);

    // Labels
    str = "Temp";
    display->drawUTF8(52 - display->getUTF8Width(str.c_str()), 11, str.c_str());
    str = "Alvo";
    display->drawUTF8(76, 11, str.c_str());

    // Escala leitura
    display->drawFrame(56, 2, 6, 32);
    tempEx = session->tempEx;
    if ( tempEx < session->tempeMin ) tempEx = session->tempeMin;
    if ( tempEx > session->tempeMax ) tempEx = session->tempeMax;
    tempeDial = round(32.0 * (tempEx - session->tempeMin) / (session->tempeMax - session->tempeMin));
    display->drawBox(56, (u8g2_uint_t)(34 - tempeDial), 6, (u8g2_uint_t)tempeDial);

    // Escala objetivo
    display->drawFrame(66, 2, 6, 32);
    tempeDial = (u8g2_uint_t)round(32.0 * (session->tempeTarget - session->tempeMin) / (session->tempeMax - session->tempeMin));
    display->drawBox(66, 34 - tempeDial, 6, tempeDial);

    // 16 pixel height
    display->setFont(u8g2_font_inb16_mn);

    // Valores leitura e objetivo
    str = String((int)session->tempEx);
    display->drawStr(52 - display->getStrWidth(str.c_str()), 34, str.c_str());
    str = String((int)session->tempeTarget);
    display->drawStr(76, 34, str.c_str());

    // 9 pixel height
    display->setFont(u8g2_font_6x13_mf);

    str = String("°C");
    display->drawUTF8((int)(round((float)(128 - display->getUTF8Width(str.c_str()))/2.0)), 46, str.c_str());

    // Status
    str = String(session->elapsed / 60) + "m" + String(session->elapsed % 60) + "s";
    if ( ! session->running() ){
      str = (session->elapsed == 0) ? "Parado" : str;
    }

    display->drawUTF8((int)(round((float)(128 - display->getUTF8Width(str.c_str()))/2.0)), 61, str.c_str());

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
 * Tela Setup
 */
scrSetup::scrSetup(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): Screen(session, display) {
  leave = NULL;
}

void scrSetup::show(){

  // 9 pixel height
  display->setFont(u8g2_font_6x13_mf);

  display->firstPage();
  do {
    display->setDrawColor(1);
    for(int i = 0; i < nitems; ++i){
      if ( highlight == i ) display->setDrawColor(0);
      else display->setDrawColor(1);
      display->drawUTF8((int)(round((float)(128 - display->getUTF8Width(labels[i]))/2.0)), (i+1)*13, labels[i]);
    }

  } while ( display->nextPage() );

}

void scrSetup::cw(){
  // Iluminar item posterior
  highlight = (highlight + 1) % nitems;
}

void scrSetup::ccw(){
  // Iluminar item anterior
  if ( --highlight < 0 ) highlight = nitems - 1;
}

Screen* scrSetup::btTopDown(){return this;}

Screen* scrSetup::btTopUp(){
  // Invocar ação correspondente ao item
  if ( highlight == 0){
    // Calibrar temperatura
    return screens[0];
  }
  else if (highlight == 1){
    // Coeficientes PID
  }
  else if (highlight == 2){
    // Parar sozinho ou Não parar sozinho
  }
  else if (highlight == 3){
    // Restaurar padrão
    session->reset();
    return leave;
  }
  return this;
}

Screen* scrSetup::btFrontDown(){return this;}

Screen* scrSetup::btFrontUp(){
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}


/***
 * Tela Calibragem
 */
scrCalib::scrCalib(Session* session, U8G2_SH1106_128X64_NONAME_2_HW_I2C* display): Screen(session, display) {
  leave = NULL;
}

void scrCalib::show(){

  // Valor formatado
  String strVal;

  // 9 pixel height
  display->setFont(u8g2_font_6x13_mf);

  display->firstPage();
  do {

    display->setDrawColor(1);
    strVal = String("Calibrar");
    display->drawUTF8((int)(round((float)(128 - display->getUTF8Width(strVal.c_str()))/2.0)), 13, strVal.c_str());

    for(int i = 0; i < nitems; ++i){
      // Mudar cor se item iluminado
      if ( highlight == i && edit < 0 ) display->setDrawColor(0);
      else display->setDrawColor(1);

      strVal = String("Temperatura ") + String(1+i);
      display->drawUTF8(0, 15+(i+1)*13, strVal.c_str());

      strVal = String((int)*(items[i].tempEx));
      strVal = strVal + " °C";

      if ( highlight == i && edit == i ) display->setDrawColor(0);
      else display->setDrawColor(1);
      display->drawUTF8(128 - display->getUTF8Width(strVal.c_str()), 15+(i+1)*13, strVal.c_str());
    }

  } while ( display->nextPage() );

}

void scrCalib::cw(){
  if ( edit < 0 ){
    // Nenhum item sendo editado, iluminar item posterior
    highlight = (highlight + 1) % nitems;
  }
  else {
    // Incrementar valor
    *(items[edit].tempEx) += 1;
    *(items[edit].tempCore) = session->tempCore;
  }
}

void scrCalib::ccw(){
  if ( edit < 0 ){
    // Nenhum item sendo editado, iluminar item anterior
    if ( --highlight < 0 ) highlight = nitems - 1;
  }
  else {
    // Decrementar valor
    *(items[edit].tempEx) -= 1;
    *(items[edit].tempCore) = session->tempCore;
  }
}

Screen* scrCalib::btTopDown(){return this;}

Screen* scrCalib::btTopUp(){
  if ( edit < 0 ){
    edit = highlight;
  }
  else {
    *(items[edit].tempCore) = session->tempCore;
    edit = -1;
  }
  return this;
}

Screen* scrCalib::btFrontDown(){return this;}

Screen* scrCalib::btFrontUp(){
  // Reconfigurar
  mat::leastsquares(3, 2, session->settings.tempCore, session->settings.tempEx, session->thCfs[1]);
  session->save();
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}
