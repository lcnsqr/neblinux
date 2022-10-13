#ifndef Screen_h
#define Screen_h

#include "display.h"
#include "session.h"

class Screen {
public:
  Screen(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);

  // Exibir tela no display
  virtual void show();

  // Rotary encoder clockwise
  virtual void cw();

  // Rotary encoder counter-clockwise
  virtual void ccw();

  // Reagir ao pressionamento dos botões
  virtual Screen *btTopDown();
  virtual Screen *btTopUp();
  virtual Screen *btFrontDown();
  virtual Screen *btFrontUp();

protected:
  Session *session;
  U8G2_SH1106_128X64_NONAME_2_HW_I2C *display;
};

class scrMain : public Screen {
public:
  scrMain(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  // void splash();
  void show();
  void cw();
  void ccw();
  Screen *btTopDown();
  Screen *btTopUp();
  Screen *btFrontDown();
  Screen *btFrontUp();

  // Tela exibida ao pressionar botão central
  Screen *leave;
};

class scrSetup : public Screen {
public:
  scrSetup(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void show();
  void cw();
  void ccw();
  Screen *btTopDown();
  Screen *btTopUp();
  Screen *btFrontDown();
  Screen *btFrontUp();

  // Texto dos itens
  const char *labels[4] = {" Calibrar sensor ", " Parar sozinho? ",
                           " Depuração ", " Restaurar padrão "};

  // Telas para itens
  Screen *screens[2];

  // Itens na tela
  int nitems;
  int highlight;

  // Tela exibida ao pressionar botão central
  Screen *leave;
};

class scrCalib : public Screen {
public:
  scrCalib(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void show();
  void cw();
  void ccw();
  Screen *btTopDown();
  Screen *btTopUp();
  Screen *btFrontDown();
  Screen *btFrontUp();

  // Tela exibida ao pressionar botão central
  Screen *leave;

  // Itens na tela
  int nitems;
  int highlight;
  int edit;

  // Valores de tempTarget para cada estágio da calibragem
  const int calibTarget[3] = {50, 150, 210};
};

class scrDebug : public Screen {
public:
  scrDebug(Session *session, U8G2_SH1106_128X64_NONAME_2_HW_I2C *display);
  void show();
  void cw();
  void ccw();
  Screen *btTopDown();
  Screen *btTopUp();
  Screen *btFrontDown();
  Screen *btFrontUp();

  // Tela exibida ao pressionar botão central
  Screen *leave;
};

#endif
