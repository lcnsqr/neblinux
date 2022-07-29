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

  // Avaliar se informações na sessão a serem
  // exibidas mudaram em relação à cópia local.
  // Se sim, atualizar a tela.
  
  // Temperatura atual
  if ( (int)session->tempeCore != (int)local.tempeCore || (int)session->tempeEx != (int)local.tempeEx ){
    local.tempeCore = session->tempeCore;
    session->changed = true;
  }

  // Contador de tempo
  if ( session->elapsed != local.elapsed ){
    local.elapsed = session->elapsed;
    session->changed = true;
  }

  // Meta de temperatura
  // O estado está em *encoderMove* no caso do rotary encoder
  if ( encoderMove >= local.encoder + 4 ) {
    local.encoder = encoderMove;
    session->cw();
    session->changed = true;
  }
  if ( encoderMove <= local.encoder - 4 ) {
    local.encoder = encoderMove;
    session->ccw();
    session->changed = true;
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

  if ( session->screen == 0 ){
    screen0();
  }
  else if ( session->screen == 1 ){
    screen1();
  }

}

void Monitor::screen0(){
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
    if ( ! session->running() ){
      str = (session->elapsed == 0) ? "Parado" : str;
    }

    display->drawUTF8((int)(round((double)(128 - display->getUTF8Width(str.c_str()))/2.0)), 61, str.c_str());

  } while ( display->nextPage() );

}

void Monitor::screen1(){

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

  } while ( display->nextPage() );

}
