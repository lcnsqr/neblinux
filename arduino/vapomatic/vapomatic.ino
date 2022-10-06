#include "task.h"
#include "session.h"
#include "therm.h"
#include "display.h"
#include "monitor.h"
#include "rotary.h"
#include "fan.h"
#include "heater.h"
#include "timer.h"
#include "shutdown.h"
#include "screen.h"

// Estado do aparelho e comunicação interserviços
Session session;
// Loop de serviços
Tasks tasks;
// Leitura da temperatura
Therm therm(&session, A0, 3*17, 17);
// Display helper library
U8G2_SH1106_128X64_NONAME_2_HW_I2C display(U8G2_R2, U8X8_PIN_NONE);
// Telas da UI
scrMain uiMain(&session, &display);
scrSetup uiSetup(&session, &display);
scrCalib uiCalib(&session, &display);
// Monitoramento de eventos
Monitor monitor(&session, &uiMain, 4, 8, 25);
// Callback de eventos do rotary encoder
Rotary rotary;
// Controle da ventoinha
Fan fan(7, &session, 75);
// Controle do aquecedor
Heater heater(5, &session, 38);
// Contador de tempo
Timer timer(&session, 1000);
// Para de encher automaticamente
Shutdown shutdown(&session, 500);

void setup() {

  // Configurar sessão
  session.load();

  // Definições de UI
  setupUI();

  // Serviços
  tasks.add(&therm);
  tasks.add(&monitor);
  tasks.add(&fan);
  tasks.add(&heater);
  tasks.add(&timer);
  tasks.add(&shutdown);

}

void loop() {
  tasks.run();
}

void setupUI() {

  // Qual tela chamar com botão frontal na tela principal
  uiMain.leave = &uiSetup;

  /**
    * Tela de setup
    */
  uiSetup.nitems = 3;
  uiSetup.screens[0] = &uiCalib;
  uiSetup.highlight = 0;
  uiSetup.leave = &uiMain;

  /**
    * Tela de calibragem
    */
  uiCalib.nitems = 3;
  uiCalib.highlight = 0;
  uiCalib.edit = -1;
  uiCalib.leave = &uiSetup;


  // Subir display
  display.begin();

}
