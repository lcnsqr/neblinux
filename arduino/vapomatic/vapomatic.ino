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
scrSplash uiSplash(&session, &display);
scrMain uiMain(&session, &display);
scrDebug uiDebug(&session, &display);
struct ScreenItem scrDebugItems[11];
// Monitoramento de eventos
Monitor monitor(&session, &uiSplash, &uiMain, 4, 8, 25);
// Callback de eventos do rotary encoder
Rotary rotary;
// Controle da ventoinha
Fan fan(&session, 7, 75);
// Controle do aquecedor
Heater heater(&session, 5, 38);
// Contador de tempo
Timer timer(&session, 1000);
// Para de encher automaticamente
Shutdown shutdown(&session, 500);

void setup() {

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

  // Valor do thermistor
  scrDebugItems[0].label = "Therm";
  scrDebugItems[0].sessionType = DBL;
  scrDebugItems[0].screenType = INT;
  scrDebugItems[0].value.d = &(session.analogTherm);

  // Temperatura na base da resistência
  scrDebugItems[1].label = "Core";
  scrDebugItems[1].sessionType = DBL;
  scrDebugItems[1].screenType = INT;
  scrDebugItems[1].value.d = &(session.tempeCore);

  // Temperatura estimada no exaustor quando ativo
  scrDebugItems[2].label = "Ex";
  scrDebugItems[2].sessionType = DBL;
  scrDebugItems[2].screenType = INT;
  scrDebugItems[2].value.d = &(session.tempeEx);

  // Temperatura alvo no exaustor
  scrDebugItems[3].label = "Alvo";
  scrDebugItems[3].sessionType = DBL;
  scrDebugItems[3].screenType = INT;
  scrDebugItems[3].value.d = &(session.tempeTarget);

  // Carga na resistência
  scrDebugItems[4].label = "Gate";
  scrDebugItems[4].sessionType = DBL;
  scrDebugItems[4].screenType = INT;
  scrDebugItems[4].value.d = &(session.PID[7]);

  // Tempo ativo
  scrDebugItems[5].label = "Tempo";
  scrDebugItems[5].sessionType = INT;
  scrDebugItems[5].screenType = INT;
  scrDebugItems[5].value.i = &(session.elapsed);

  // Indicadores de encerramento
  scrDebugItems[6].label = "e0";
  scrDebugItems[6].sessionType = DBL;
  scrDebugItems[6].screenType = DBL;
  scrDebugItems[6].value.d = &(session.end[0]);

  scrDebugItems[7].label = "e1";
  scrDebugItems[7].sessionType = DBL;
  scrDebugItems[7].screenType = DBL;
  scrDebugItems[7].value.d = &(session.end[1]);

  // Controlador PID
  scrDebugItems[8].label = "P";
  scrDebugItems[8].sessionType = DBL;
  scrDebugItems[8].screenType = DBL;
  scrDebugItems[8].value.d = &(session.PID[0]);

  scrDebugItems[9].label = "I";
  scrDebugItems[9].sessionType = DBL;
  scrDebugItems[9].screenType = DBL;
  scrDebugItems[9].value.d = &(session.PID[1]);

  scrDebugItems[10].label = "D";
  scrDebugItems[10].sessionType = DBL;
  scrDebugItems[10].screenType = DBL;
  scrDebugItems[10].value.d = &(session.PID[2]);


  uiDebug.nitems = 11;
  uiDebug.items = scrDebugItems;

  uiDebug.leave = &uiMain;

  uiMain.leave = &uiDebug;

  display.begin();

}
