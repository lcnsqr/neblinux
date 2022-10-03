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

// Configurações
struct Settings settings;
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
scrCalib uiCalib(&session, &display);
struct ScreenItem scrCalibItems[3];
// Monitoramento de eventos
Monitor monitor(&session, &uiMain, 4, 8, 25);
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

  // Configurar sessão
  session.load(&settings);

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
  uiMain.leave = &uiCalib;

  /**
    * Itens na tela de calibragem
    */
  // Valor do thermistor
  scrCalibItems[0].label = "Leitura A";
  scrCalibItems[0].sessionType = DBL;
  scrCalibItems[0].screenType = INT;
  scrCalibItems[0].value.d = &(session.analogTherm);

  // Temperatura na base da resistência
  scrCalibItems[1].label = "Leitura B";
  scrCalibItems[1].sessionType = DBL;
  scrCalibItems[1].screenType = INT;
  scrCalibItems[1].value.d = &(session.tempeCore);

  // Temperatura estimada no exaustor quando ativo
  scrCalibItems[2].label = "Leitura C";
  scrCalibItems[2].sessionType = DBL;
  scrCalibItems[2].screenType = INT;
  scrCalibItems[2].value.d = &(session.tempeEx);

  uiCalib.nitems = 3;
  uiCalib.items = scrCalibItems;
  uiCalib.highlight = 0;
  uiCalib.edit = -1;
  uiCalib.leave = &uiMain;


  // Subir display
  display.begin();

}
