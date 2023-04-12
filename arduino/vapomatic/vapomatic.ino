#include "display.h"
#include "fan.h"
#include "heater.h"
#include "monitor.h"
#include "rotary.h"
#include "screen.h"
#include "session.h"
#include "autostop.h"
#include "state.h"
#include "task.h"
#include "therm.h"

// Estado do aparelho e comunicação interserviços
Session session;
// Controle do aquecedor
Heater heater(5, &session, 38);
// Loop de serviços
Tasks tasks;
// Leitura da temperatura
Therm therm(&session, A0, Therm::bufLen * 17);
// Display helper library
U8G2_SH1106_128X64_NONAME_2_HW_I2C display(U8G2_R2, U8X8_PIN_NONE);
// Telas da UI
scrMain uiMain(&session, &display);
scrSetup uiSetup(&session, &display);
// scrCalib uiCalib(&session, &display);
// scrPID uiPID(&session, &display);
// Monitoramento de eventos
Monitor monitor(&session, &uiMain, 4, 8, 25);
// Callback de eventos do rotary encoder
Rotary rotary;
// Controle da ventoinha
Fan fan(7, &session, 75);
// Para de encher automaticamente
// Três pontos amostrais separados por 3.5s
// O declive importante aparece num intervalo ~ 10s
Autostop autostop(&session, 3500);

void setup() {

  Serial.begin(115200);

  // Configurar sessão
  session.load();

  // Definições de UI
  setupUI();

  // Serviços (alterar quantidade em task.h)
  tasks.tasks[0] = &therm;
  tasks.tasks[1] = &monitor;
  tasks.tasks[2] = &fan;
  tasks.tasks[3] = &heater;
  tasks.tasks[4] = &autostop;
}

void loop() { tasks.run(); }

void setupUI() {

  // Qual tela chamar com botão frontal na tela principal
  uiMain.leave = &uiSetup;

  /**
   * Tela de setup
   */
  uiSetup.nitems = 4;
  // uiSetup.screens[0] = &uiCalib;
  // uiSetup.screens[1] = &uiPID;
  uiSetup.highlight = 0;
  uiSetup.leave = &uiMain;

  /**
   * Tela de calibragem
   */
  // uiCalib.nitems = 3;
  // uiCalib.highlight = 0;
  // uiCalib.edit = -1;
  // uiCalib.leave = &uiSetup;

  /**
   * Configurações do PID
   */
  // uiPID.nitems = 3;
  // uiPID.highlight = 0;
  // uiPID.edit = -1;
  // uiPID.leave = &uiSetup;

  // Subir display
  display.begin();
}
