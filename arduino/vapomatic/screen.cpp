#include "screen.h"
#include "display.h"
#include "mat.h"
#include "session.h"

Screen::Screen(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display)
    : session(session), display(display) {
  leave = NULL;
}

void Screen::h1Setup(String &str) {
  str = String((int)session->state.PID[4]) + " / " + String((int)session->state.tempCore) +
        " °C / " + String((int)session->state.tempEx) + " °C";
  display->drawUTF8(
      (int)(round((float)(128 - display->getUTF8Width(str.c_str())) / 2.0)), 13,
      str.c_str());
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

  if (millis() < 2000) {
    splash();
    return;
  }

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
    tempEx = session->state.tempEx;
    if (tempEx < session->tempMin)
      tempEx = session->tempMin;
    if (tempEx > session->tempMax)
      tempEx = session->tempMax;
    tempDial = round(32.0 * (tempEx - session->tempMin) /
                     (session->tempMax - session->tempMin));
    display->drawBox(56, (u8g2_uint_t)(34 - tempDial), 6,
                     (u8g2_uint_t)tempDial);

    // Escala objetivo
    display->drawFrame(66, 2, 6, 32);
    tempDial =
        (u8g2_uint_t)round(32.0 * (session->state.tempTarget - session->tempMin) /
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
    str = String(session->state.elapsed / 60) + "m" + String(session->state.elapsed % 60) +
          "s";
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
    if (session->state.tempTarget + 10 > session->tempMax)
      return;
    session->state.tempTarget += 10;
  } else {
    if (session->state.tempTarget - 10 < session->tempMin)
      return;
    session->state.tempTarget -= 10;
  }
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
  const char *labels[4] = {" Calibrar sensor ", " Coeficientes PID ",
                           " Parar sozinho? ", " Restaurar padrão "};

  // 9 pixel height
  display->setFont(u8g2_font_6x13_mf);

  display->firstPage();
  do {
    display->setDrawColor(1);
    for (int i = 0; i < nitems; ++i) {
      if (highlight == i)
        display->setDrawColor(0);
      else
        display->setDrawColor(1);
      if (i == 2) {
        // Auto shutdown
        String label =
            String(labels[i]) +
            String((session->settings.shutEnabled) ? "Sim " : "Não ");
        display->drawUTF8(
            (int)(round((float)(128 - display->getUTF8Width(label.c_str())) /
                        2.0)),
            (i + 1) * 13, label.c_str());
      } else
        display->drawUTF8(
            (int)(round((float)(128 - display->getUTF8Width(labels[i])) / 2.0)),
            (i + 1) * 13, labels[i]);
    }

  } while (display->nextPage());
}

void scrSetup::rotate(const char forward) {
  if (forward) {
    // Iluminar item posterior
    highlight = (highlight + 1) % nitems;
  } else {
    // Iluminar item anterior
    if (--highlight < 0)
      highlight = nitems - 1;
  }
}

Screen *scrSetup::btTop() {
  // Invocar ação correspondente ao item
  if (highlight == 0) {
    // Calibrar temperatura
    return screens[0];
  } else if (highlight == 1) {
    // Ajustar PID
    return screens[1];
  } else if (highlight == 2) {
    // Alternar desligamento automático
    session->settings.shutEnabled = !session->settings.shutEnabled;
    session->save();
    session->changed = true;
  } else if (highlight == 3) {
    // Restaurar padrão
    session->reset();
    return leave;
  }
  return this;
}

Screen *scrSetup::btFront() {
  // Chamar a tela definida em leave
  session->changed = true;
  return leave;
}

/***
 * Tela Calibragem
 */
//scrCalib::scrCalib(Session *session,
//                   U8G2_SH1106_128X64_NONAME_2_HW_I2C *display)
//    : Screen(session, display) {}
//
//void scrCalib::show() {
//
//  // Valor formatado
//  String strVal;
//
//  // 9 pixel height
//  display->setFont(u8g2_font_6x13_mf);
//
//  display->firstPage();
//  do {
//
//    display->setDrawColor(1);
//    h1Setup(strVal);
//
//    for (int i = 0; i < nitems; ++i) {
//      // Mudar cor se item iluminado
//      if (highlight == i && edit < 0)
//        display->setDrawColor(0);
//      else
//        display->setDrawColor(1);
//
//      strVal = String(calibTarget[i]) + " °C";
//      display->drawUTF8(0, 15 + (i + 1) * 13, strVal.c_str());
//
//      display->setDrawColor(1);
//      display->drawUTF8(
//          (int)(round((float)(128 - display->getUTF8Width("=")) / 2.0)),
//          15 + (i + 1) * 13, "=");
//
//      strVal = String((int)session->settings.tempEx[i]) + " °C";
//
//      if (highlight == i && edit == i)
//        display->setDrawColor(0);
//      else
//        display->setDrawColor(1);
//      display->drawUTF8(128 - display->getUTF8Width(strVal.c_str()),
//                        15 + (i + 1) * 13, strVal.c_str());
//    }
//
//  } while (display->nextPage());
//}
//
//void scrCalib::rotate(const char forward) {
//  if (forward) {
//    if (edit < 0) {
//      // Nenhum item sendo editado, iluminar item posterior
//      highlight = (highlight + 1) % nitems;
//    } else {
//      // Ajustar temperatura estimada conforme probe externo
//      session->settings.tempEx[edit] += 5;
//      // Registrar qual é a temperatura interna ao final da aferição
//      session->settings.tempCore[edit] = session->state.tempCore;
//      // Reconfigurar
//      mat::leastsquares(3, 2, session->settings.tempCore,
//                        session->settings.tempEx, session->state.thCfs[1]);
//    }
//  } else {
//    if (edit < 0) {
//      // Nenhum item sendo editado, iluminar item anterior
//      if (--highlight < 0)
//        highlight = nitems - 1;
//    } else {
//      // Ajustar temperatura estimada conforme probe externo
//      session->settings.tempEx[edit] -= 5;
//      // Registrar qual é a temperatura interna ao final da aferição
//      session->settings.tempCore[edit] = session->state.tempCore;
//      // Reconfigurar
//      mat::leastsquares(3, 2, session->settings.tempCore,
//                        session->settings.tempEx, session->state.thCfs[1]);
//    }
//  }
//}
//
//Screen *scrCalib::btTop() {
//
//  if (edit < 0) {
//    edit = highlight;
//    // Qual temperatura aferir
//    session->state.tempTarget = calibTarget[edit];
//    // Aquecer e aferir para o nível correspondente
//    session->start();
//  } else {
//    session->settings.tempCore[edit] = session->state.tempCore;
//    // Desligar
//    session->stop();
//    // Sair da edição
//    edit = -1;
//  }
//  return this;
//}
//
//Screen *scrCalib::btFront() {
//  session->save();
//  // Chamar a tela definida em leave
//  session->changed = true;
//  return leave;
//}
//
///***
// * Tela para alterar coeficientes do PID
// */
//scrPID::scrPID(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display)
//    : Screen(session, display) {}
//
//void scrPID::show() {
//
//  // Texto dos itens
//  const char *labels[3] = {"P", "I", "D"};
//
//  // Valor formatado
//  String strVal;
//
//  // 9 pixel height
//  display->setFont(u8g2_font_6x13_mf);
//
//  display->firstPage();
//  do {
//
//    display->setDrawColor(1);
//    h1Setup(strVal);
//
//    for (int i = 0; i < nitems; ++i) {
//      display->setDrawColor(1);
//      strVal = String(session->state.PID[i]);
//      display->drawUTF8(0, 15 + (i + 1) * 13, strVal.c_str());
//
//      // Mudar cor se item iluminado
//      if (highlight == i && edit < 0)
//        display->setDrawColor(0);
//      else
//        display->setDrawColor(1);
//      display->drawUTF8(
//          (int)(round((float)(128 - display->getUTF8Width(labels[i])) / 2.0)),
//          15 + (i + 1) * 13, labels[i]);
//
//      strVal = String(session->settings.PID[i], 4);
//
//      if (highlight == i && edit == i)
//        display->setDrawColor(0);
//      else
//        display->setDrawColor(1);
//      display->drawUTF8(128 - display->getUTF8Width(strVal.c_str()),
//                        15 + (i + 1) * 13, strVal.c_str());
//    }
//
//  } while (display->nextPage());
//}
//
//void scrPID::rotate(const char forward) {
//  if (forward) {
//    if (edit < 0) {
//      // Nenhum item sendo editado, iluminar item posterior
//      highlight = (highlight + 1) % nitems;
//    } else {
//      // Ajustar coeficiente
//      session->settings.PID[edit] += (edit == 1) ? 1e-4 : 1e-2;
//    }
//  } else {
//    if (edit < 0) {
//      // Nenhum item sendo editado, iluminar item anterior
//      if (--highlight < 0)
//        highlight = nitems - 1;
//    } else {
//      // Ajustar coeficiente
//      session->settings.PID[edit] -= (edit == 1) ? 1e-4 : 1e-2;
//    }
//  }
//}
//
//Screen *scrPID::btTop() {
//
//  if (edit < 0) {
//    edit = highlight;
//  } else {
//    // Sair da edição
//    edit = -1;
//  }
//  return this;
//}
//
//Screen *scrPID::btFront() {
//  session->save();
//  // Chamar a tela definida em leave
//  session->changed = true;
//  return leave;
//}
