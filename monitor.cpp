#include "task.h"
#include "monitor.h"
#include "session.h"
#include "display.h"

extern long int encoderMove;

Monitor::Monitor(Session* session, Adafruit_SSD1306* display, unsigned long wait): Task(wait), session(session), display(display) {
  
  local = *session;

}

void Monitor::begin(){
  //Serial.begin(115200);
  //Serial.println("Start");

  if ( ! display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS) ) {
    //Serial.println("Display error");
    for(;;);
  }

  display->cp437(true);

  // Orientação invertida do display
  display->setRotation(2);
}

void Monitor::action(){
  
  if ( session->temperature != local.temperature ){
    local.temperature = session->temperature;
    session->changed = true;
  }

  if ( encoderMove >= local.encoder + 4 ) {
    local.encoder = encoderMove;
    session->cw();
  }

  if ( encoderMove <= local.encoder - 4 ) {
    local.encoder = encoderMove;
    session->ccw();
  }
  
  if ( session->changed ) show();

}

void Monitor::show(){
  display->clearDisplay();

  display->setCursor(0,0); // Posição x,y

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);

  display->print("Temp: ");
  display->println((int)session->temperature);

  //display->write(248); // Sinal de grau
  //display->println("C");

  display->print("Alvo: ");
  display->println((int)session->tempeTarget);

  display->println("P I D");
  display->print(session->PID[0]);
  display->print(" ");
  display->print(session->PID[1]);
  display->print(" ");
  display->println(session->PID[2]);

  display->print("F: ");
  display->println((int)session->PID[7]);

  //display->setTextSize(1);
  //display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  if ( session->on ){
    display->print("Ligado");
  }
  else {
    display->print("Desligado");

    // Reset PID
    session->PID[0] = 0;
    session->PID[1] = 0;
    session->PID[2] = 0;
    session->PID[3] = 0;
    session->PID[7] = 0;
  }

  display->display();

}
