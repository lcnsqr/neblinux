#ifndef Screen_h
#define Screen_h

#include "display.h"
#include "session.h"

class Screen {
public:
  Screen(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);

  // Exibir tela no display
  virtual void show();

  // Rotary encoder
  virtual void rotate(const char forward);

  // Reagir ao pressionamento dos botões
  virtual Screen *btTop();
  virtual Screen *btFront();

  // Tela exibida ao pressionar botão central
  Screen *leave;

  // Elementos comuns de UI
  void h1Setup(String &str);

protected:
  Session *session;
  U8G2_SH1106_128X64_NONAME_2_HW_I2C *display;
};

class scrMain : public Screen {
public:
  scrMain(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void splash();
  void show();
  void rotate(const char forward);
  Screen *btTop();
  Screen *btFront();
};

class scrSetup : public Screen {
public:
  scrSetup(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void show();
  void rotate(const char forward);
  Screen *btTop();
  Screen *btFront();

  // Telas para itens
  Screen *screens[2];

  // Itens na tela
  int nitems;
  int highlight;
};

class scrCalib : public Screen {
public:
  scrCalib(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void show();
  void rotate(const char forward);
  Screen *btTop();
  Screen *btFront();

  // Itens na tela
  int nitems;
  int highlight;
  int edit;

  // Valores de tempTarget para cada estágio da calibragem
  const int calibTarget[3] = {50, 150, 210};
};

class scrPID : public Screen {
public:
  scrPID(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void show();
  void rotate(const char forward);
  Screen *btTop();
  Screen *btFront();

  // Itens na tela
  int nitems;
  int highlight;
  int edit;
};

#endif
