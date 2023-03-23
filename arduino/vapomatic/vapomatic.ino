#include "display.h"
#include "fan.h"
#include "heater.h"
#include "monitor.h"
#include "rotary.h"
#include "screen.h"
#include "state.h"
#include "session.h"
#include "shutdown.h"
#include "task.h"
#include "therm.h"

// Estado do aparelho e comunicação interserviços
Session session;
// Loop de serviços
Tasks tasks;
// Leitura da temperatura
Therm therm(&session, A0, Therm::bufLen * 17);
// Display helper library
U8G2_SH1106_128X64_NONAME_2_HW_I2C display(U8G2_R2, U8X8_PIN_NONE);
// Telas da UI
scrMain uiMain(&session, &display);
scrSetup uiSetup(&session, &display);
//scrCalib uiCalib(&session, &display);
//scrPID uiPID(&session, &display);
// Monitoramento de eventos
Monitor monitor(&session, &uiMain, 4, 8, 25);
// Callback de eventos do rotary encoder
Rotary rotary;
// Controle da ventoinha
Fan fan(7, &session, 75);
// Controle do aquecedor
Heater heater(5, &session, 38);
// Para de encher automaticamente
// Três pontos amostrais separados por 3.5s
// O declive importante aparece num intervalo ~ 10s
Shutdown shutdown(&session, 3500);

// Intervalo de transmissão serial
#define serial_wait 250
long int serial_before, serial_now;

// Buffer de recepção do estado
struct StateIO stateIn;

void setup() {

  Serial.begin(115200);
  serial_before = millis();

  // Configurar sessão
  session.load();

  // Definições de UI
  setupUI();

  // Serviços (alterar quantidade em task.h)
  tasks.tasks[0] = &therm;
  tasks.tasks[1] = &monitor;
  tasks.tasks[2] = &fan;
  tasks.tasks[3] = &heater;
  tasks.tasks[4] = &shutdown;

}

void loop() {
  tasks.run();
  serial_now = millis();
  if ( serial_now - serial_before >= serial_wait ){
    Serial.write((char*)&(session.state), sizeof(struct State));
    serial_before = serial_now;
  }

  if (Serial.available() > 0) {
    Serial.readBytes((char*)&stateIn, sizeof(struct StateIO));
    delay(0.1);
    session.state.tempTarget = stateIn.tempTarget;
    session.state.on = stateIn.on;
    session.state.fan = stateIn.fan;
    session.state.PID[5] = (float)stateIn.PID_enabled;
    if (session.state.PID[5] == 0)
      session.state.PID[4] = stateIn.heat;
  }

}

void setupUI() {

  // Qual tela chamar com botão frontal na tela principal
  uiMain.leave = &uiSetup;

  /**
   * Tela de setup
   */
  uiSetup.nitems = 4;
  //uiSetup.screens[0] = &uiCalib;
  //uiSetup.screens[1] = &uiPID;
  uiSetup.highlight = 0;
  uiSetup.leave = &uiMain;

  /**
   * Tela de calibragem
   */
  //uiCalib.nitems = 3;
  //uiCalib.highlight = 0;
  //uiCalib.edit = -1;
  //uiCalib.leave = &uiSetup;

  /**
   * Configurações do PID
   */
  //uiPID.nitems = 3;
  //uiPID.highlight = 0;
  //uiPID.edit = -1;
  //uiPID.leave = &uiSetup;

  // Subir display
  display.begin();
}
