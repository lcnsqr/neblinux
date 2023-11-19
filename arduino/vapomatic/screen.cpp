#include "screen.h"
#include "display.h"
#include "session.h"

Screen::Screen(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display)
    : session(session), display(display) {
  leave = NULL;
}

void Screen::h1Setup(String &str) {
  str = String((int)session->state.PID[4]) + " / " +
        String((int)session->state.tempCore) + " °C / " +
        String((int)session->state.tempEx) + " °C";
  display->drawUTF8(
      (int)(round((float)(128 - display->getUTF8Width(str.c_str())) / 2.0)), 13,
      str.c_str());
}

void Screen::clear() { display->clear(); }

void Screen::saver() {

  display->firstPage();
  do {

    display->drawDisc(session->ss.pos[0], session->ss.pos[1], 1);

  } while (display->nextPage());

  if ((session->ss.pos[0] + session->ss.dir[0]) <= 0 ||
      (session->ss.pos[0] + session->ss.dir[0]) >= display->getDisplayWidth())
    session->ss.dir[0] *= -1;

  if ((session->ss.pos[1] + session->ss.dir[1]) <= 0 ||
      (session->ss.pos[1] + session->ss.dir[1]) >= display->getDisplayHeight())
    session->ss.dir[1] *= -1;

  session->ss.pos[0] += session->ss.dir[0];
  session->ss.pos[1] += session->ss.dir[1];

  // Mudar direção
  if (millis() % 51 == 0) {
    int d1 = -session->ss.dir[1];
    session->ss.dir[1] = session->ss.dir[0];
    session->ss.dir[0] = d1;
  }
}

/***
 * Tela principal
 */
scrMain::scrMain(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display)
    : Screen(session, display) {}

void scrMain::splash() {

  static const unsigned char logo_bits[] U8X8_PROGMEM = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00,
      0x00, 0x00, 0xf8, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x01, 0x00,
      0x00, 0xe0, 0xff, 0xff, 0x03, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x03, 0x00,
      0x80, 0xff, 0xff, 0xff, 0x07, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00,
      0xf8, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x1f, 0x00,
      0xf8, 0xff, 0x03, 0xf8, 0x3f, 0x00, 0xf8, 0xff, 0x00, 0xe0, 0x7f, 0x00,
      0xf8, 0x3f, 0x00, 0xc0, 0x7f, 0x00, 0xf8, 0x1f, 0x00, 0x80, 0xff, 0x00,
      0xf8, 0x0f, 0x00, 0x00, 0xff, 0x01, 0xf8, 0x0f, 0x00, 0x00, 0xfe, 0x01,
      0xf8, 0x07, 0x00, 0x00, 0xfe, 0x03, 0xf8, 0x07, 0x00, 0x00, 0xfc, 0x07,
      0xf8, 0x03, 0x00, 0x00, 0xfc, 0x0f, 0xf8, 0x03, 0x00, 0x00, 0xfc, 0x0f,
      0xf8, 0x03, 0x00, 0x00, 0xfc, 0x1f, 0xf8, 0x03, 0x00, 0x00, 0xfc, 0x1f,
      0xf8, 0x03, 0x00, 0x00, 0xfc, 0x1f, 0xf8, 0x03, 0x00, 0x00, 0xfc, 0x1f,
      0xf8, 0x03, 0x00, 0x00, 0xfc, 0x0f, 0xf8, 0x03, 0x00, 0x00, 0xfc, 0x0f,
      0xf8, 0x07, 0x00, 0x00, 0xfc, 0x07, 0xf8, 0x07, 0x00, 0x00, 0xfe, 0x03,
      0xf8, 0x0f, 0x00, 0x00, 0xfe, 0x01, 0xf8, 0x0f, 0x00, 0x00, 0xff, 0x01,
      0xf8, 0x1f, 0x00, 0x80, 0xff, 0x00, 0xf8, 0x3f, 0x00, 0xc0, 0x7f, 0x00,
      0xf8, 0xff, 0x00, 0xe0, 0x7f, 0x00, 0xf8, 0xff, 0x03, 0xf8, 0x3f, 0x00,
      0xf8, 0xff, 0xff, 0xff, 0x1f, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x0f, 0x00,
      0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0xff, 0xff, 0xff, 0x07, 0x00,
      0x00, 0xfc, 0xff, 0xff, 0x03, 0x00, 0x00, 0xe0, 0xff, 0xff, 0x03, 0x00,
      0x00, 0x00, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x00, 0x00,
      0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  // 9 pixel height
  display->setFont(u8g2_font_6x13_mf);
  display->setDrawColor(1);

  String str;

  display->firstPage();
  do {

    display->drawXBMP(40, 0, 48, 48, logo_bits);

    str = String("VAPOMATIC");
    display->drawUTF8(
        (int)(round((float)(128 - display->getUTF8Width(str.c_str())) / 2.0)),
        62, str.c_str());

  } while (display->nextPage());
}

void scrMain::show() {

  if (session->state.splash) {
    splash();
    return;
  }

  // Evitar ficar parado na splash screen até que algum evento ocorra
  session->changed = true;

  // String para exibir labels e valores
  String str;

  // Valor na escala de temperaturas
  u8g2_uint_t tempDial;

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
    tempEx =
        constrain(session->state.tempEx, session->tempMin, session->tempMax);
    tempDial = round(32.0 * (tempEx - session->tempMin) /
                     (session->tempMax - session->tempMin));
    display->drawBox(56, (u8g2_uint_t)(34 - tempDial), 6,
                     (u8g2_uint_t)tempDial);
    // Escala objetivo
    display->drawFrame(66, 2, 6, 32);
    tempDial =
        (u8g2_uint_t)round(32.0 *
                           (constrain(session->state.tempTarget,
                                      session->tempMin, session->tempMax) -
                            session->tempMin) /
                           (session->tempMax - session->tempMin));
    display->drawBox(66, 34 - tempDial, 6, tempDial);

    // 16 pixel height
    display->setFont(u8g2_font_inb16_mn);

    // Valores leitura e objetivo
    str = String((int)tempEx);
    display->drawStr(52 - display->getStrWidth(str.c_str()), 34, str.c_str());
    str = String((int)session->state.tempTarget);
    display->drawStr(76, 34, str.c_str());

    // 9 pixel height
    display->setFont(u8g2_font_6x13_mf);

    str = String("°C");
    display->drawUTF8(
        (int)(round((float)(128 - display->getUTF8Width(str.c_str())) / 2.0)),
        46, str.c_str());

    // Status
    str = String(session->state.elapsed / 60) + "m" +
          String(session->state.elapsed % 60) + "s";
    if (!session->running()) {
      str = (session->state.elapsed == 0) ? "VAPOMATIC" : str;
    }

    display->drawUTF8(
        (int)(round((float)(128 - display->getUTF8Width(str.c_str())) / 2.0)),
        62, str.c_str());

  } while (display->nextPage());
}

void scrMain::rotate(const char forward) {
  if (forward) {
    if (session->state.tempTarget + session->state.tempStep > session->tempMax)
      return;
    session->state.tempTarget += session->state.tempStep;
  } else {
    if (session->state.tempTarget - session->state.tempStep < session->tempMin)
      return;
    session->state.tempTarget -= session->state.tempStep;
  }
  session->state.targetLastChange = millis() / 1000;
}

Screen *scrMain::btTop() {
  if (!session->running()) {
    session->start();
  } else {
    session->stop();
  }
  return this;
}

Screen *scrMain::btFront() {
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}

/***
 * Tela Setup
 */
scrSetup::scrSetup(Session *session,
                   U8G2_SH1106_128X64_NONAME_2_HW_I2C *display)
    : Screen(session, display) {}

void scrSetup::show() {

  // Texto dos itens
  const char *labels[3] = {" Parar sozinho? ", " Passo do giro  ",
                           " Número serial: "};

  String strVal;

  // 9 pixel height
  display->setFont(u8g2_font_6x13_mf);

  display->firstPage();
  do {
    display->setDrawColor(1);
    for (int i = 0; i < nitems; ++i) {

      if (highlight == i && edit < 0)
        display->setDrawColor(0);
      else
        display->setDrawColor(1);

      if (i == 0) {

        // Auto stop
        display->drawUTF8(0, (i + 1) * 13, labels[i]);
        strVal = String((session->state.autostop) ? "Sim " : "Não ");
        display->setDrawColor(1);
        display->drawUTF8(128 - display->getUTF8Width(strVal.c_str()),
                          (i + 1) * 13, strVal.c_str());

      } else if (i == 1) {

        // Passo do giro
        display->drawUTF8(0, (i + 1) * 13, labels[i]);

        if (highlight == i && edit == i)
          display->setDrawColor(0);
        else
          display->setDrawColor(1);

        strVal = String(" ") + String(session->state.tempStep) + String(" ");
        display->drawUTF8(128 - display->getUTF8Width(strVal.c_str()),
                          (i + 1) * 13, strVal.c_str());

      } else if (i == 2) {
        // Label serial
        display->drawUTF8(
            (int)(round((float)(128 - display->getUTF8Width(labels[i])) / 2.0)),
            (i + 1) * 13, labels[i]);
      } else if (i == 3) {
        // Serial
        strVal = String(" ") + String(session->state.serial) + String(" ");
        display->drawUTF8(
            (int)(round((float)(128 - display->getUTF8Width(strVal.c_str())) /
                        2.0)),
            (i + 1) * 13, strVal.c_str());
      }
    }

  } while (display->nextPage());
}

void scrSetup::rotate(const char forward) {
  if (forward) {
    if (edit < 0) {
      // Nenhum item sendo editado, iluminar item posterior
      highlight = (highlight + 1) % (nitems - 2);
    } else if (edit == 1) {
      // Ajustar passo do giro
      if (session->state.tempStep == 5)
        session->state.tempStep = 10;
      if (session->state.tempStep == 1)
        session->state.tempStep = 5;
      if (session->state.tempStep <= 0)
        session->state.tempStep = 1;
    }
  } else {
    if (edit < 0) {
      // Nenhum item sendo editado, iluminar item anterior
      if (--highlight < 0)
        highlight = (nitems - 2) - 1;
    } else {
      // Ajustar passo do giro
      if (session->state.tempStep == 5)
        session->state.tempStep = 1;
      if (session->state.tempStep == 10)
        session->state.tempStep = 5;
      if (session->state.tempStep >= 11)
        session->state.tempStep = 10;
    }
  }
}

Screen *scrSetup::btTop() {
  // Invocar ação correspondente ao item
  if (highlight == 0) {
    // Alternar desligamento automático
    session->state.autostop = !session->state.autostop;
    session->changed = true;
  } else if (highlight == 1) {
    // Ajustar passo do giro
    if (edit < 0) {
      edit = highlight;
    } else {
      // Sair da edição
      edit = -1;
    }
  }
  return this;
}

Screen *scrSetup::btFront() {
  session->save();
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}
