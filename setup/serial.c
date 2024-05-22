#include "setup.h"

// Atualização da cópia local do estado
void *pthread_updateState(void *arg) {

  struct Global *glb = (struct Global*)arg;

  glb->portDevice = open(glb->pathDevice, O_RDWR);

  int init_tty_return = init_tty(glb->portDevice, B115200);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n", glb->pathDevice, init_tty_return);
    pthread_exit((void *)NULL);
  }

  // Wait until serial port and device are ready
  sleep(3);

  while (!glb->exit) {
    deviceStateRead(glb);
    usleep(250e+3);
  }

  close(glb->portDevice);

  pthread_exit((void *)NULL);
}

// Atualizar informações da sonda
void *pthread_updateProbe(void *arg) {

  struct Global *glb = (struct Global*)arg;

  glb->portProbe = open(glb->pathProbe, O_RDWR);

  int init_tty_return = init_tty(glb->portProbe, B115200);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n", glb->pathProbe, init_tty_return);
  }

  // Wait until serial port and device are ready
  sleep(3);

  while (!glb->exit) {
    probeRead_MAX6675(glb);
    //probeRead_TA612c()int portDevice;
    usleep(250e+3);
  }

  close(glb->portProbe);

  pthread_exit((void *)NULL);
}

// One byte commands
void deviceCmd(char cmd, struct Global *glb){
  // Flushes both data received but not read,
  // and data written but not transmitted.
  pthread_mutex_lock(&(glb->device_mut));
  tcflush(glb->portDevice, TCIOFLUSH);
  write(glb->portDevice, &cmd, 1);
  pthread_mutex_unlock(&(glb->device_mut));
}

void probeRead_TA612c(struct Global *glb){

  // Estrutura de escrita para o aparelho
  char writeOut[5];

  // Estrutura de leitura do aparelho
  char readIn[13];

  // Comando de leitura
  writeOut[0] = 0xAA;
  writeOut[1] = 0x55;
  writeOut[2] = 0x01;
  writeOut[3] = 0x03;
  writeOut[4] = 0x03;

  int rx_bytes = 0;

  pthread_mutex_lock(&(glb->probe_mut));
  write(glb->portProbe, writeOut, 5);
  rx_bytes = read(glb->portProbe, readIn, 13);
  pthread_mutex_unlock(&(glb->probe_mut));

  // Decodificar temperaturas
  pthread_mutex_lock(&(glb->tempProbe_mut));
  glb->tempProbe[0] = (float)((readIn[5] << 8) | (readIn[4] & 0xff)) / 10.0;
  glb->tempProbe[1] = (float)((readIn[7] << 8) | (readIn[6] & 0xff)) / 10.0;
  glb->tempProbe[2] = (float)((readIn[9] << 8) | (readIn[8] & 0xff)) / 10.0;
  glb->tempProbe[3] = (float)((readIn[11] << 8) | (readIn[10] & 0xff)) / 10.0;
  pthread_mutex_unlock(&(glb->tempProbe_mut));

}

void probeRead_MAX6675(struct Global *glb){

  // Comando de leitura
  char cmd = SERIAL_READ;

  // Resposta da sonda
  float reply;

  int rx_bytes = 0;

  pthread_mutex_lock(&(glb->probe_mut));
  tcflush(glb->portProbe, TCIOFLUSH);
  write(glb->portProbe, &cmd, 1);
  pthread_mutex_unlock(&(glb->probe_mut));
  usleep(80e+3);
  pthread_mutex_lock(&(glb->probe_mut));
  rx_bytes = read(glb->portProbe, &reply, sizeof(float));
  pthread_mutex_unlock(&(glb->probe_mut));

  pthread_mutex_lock(&(glb->tempProbe_mut));
  for (int i = 0; i < 4; ++i)
    glb->tempProbe[i] = reply;
  pthread_mutex_unlock(&(glb->tempProbe_mut));

}

// Leitura do estado no aparelho
void deviceStateRead(struct Global *glb){

  // Cópia local temporária do estado recebido
  struct State stateLocal;

  int rx_bytes = 0;

  const char cmd = SERIAL_READ;

  while (1){

    // Read the state
    rx_bytes = 0;

    // Preparar checagem da transmissão
    stateLocal.serialCheck = ~ SERIAL_TAG;

    pthread_mutex_lock(&(glb->device_mut));

    tcflush(glb->portDevice, TCIOFLUSH);
    write(glb->portDevice, &cmd, 1);

    usleep(100e+3);

    for (int b = 0; b < sizeof(struct State); b += 4)
      rx_bytes = read(glb->portDevice, (char*)&stateLocal + b, 4);

    pthread_mutex_unlock(&(glb->device_mut));

    usleep(100e+3);
    
    // Repetir se houver inconsistência no recebimento
    if ( stateLocal.serialCheck == SERIAL_TAG ){
      break;
    }
    else {
      usleep(200e+3);
    }
  }

  // Copiar estado para a estrutura principal
  pthread_mutex_lock(&(glb->state_mut));
  memcpy(&(glb->state), &stateLocal, sizeof(struct State));
  pthread_mutex_unlock(&(glb->state_mut));

  // Atualizar a estrutura de envio com o estado recebido
  pthread_mutex_lock(&(glb->stateOut_mut));
  glb->stateOut.tempTarget = stateLocal.tempTarget;
  glb->stateOut.tempStep = stateLocal.tempStep;
  glb->stateOut.fan = stateLocal.fan;
  glb->stateOut.PID_enabled = stateLocal.PID_enabled;
  glb->stateOut.heat = (stateLocal.PID_enabled) ? 0 : stateLocal.cPID[4];
  glb->stateOut.cTemp[0] = stateLocal.cTemp[0];
  glb->stateOut.cTemp[1] = stateLocal.cTemp[1];
  glb->stateOut.cTemp[2] = stateLocal.cTemp[2];
  glb->stateOut.cTemp[3] = stateLocal.cTemp[3];
  glb->stateOut.cPID[0] = stateLocal.cPID[0];
  glb->stateOut.cPID[1] = stateLocal.cPID[1];
  glb->stateOut.cPID[2] = stateLocal.cPID[2];
  glb->stateOut.autostop = stateLocal.autostop;
  glb->stateOut.screensaver = stateLocal.screensaver;
  glb->stateOut.cStop[0] = stateLocal.cStop[0];
  glb->stateOut.cStop[1] = stateLocal.cStop[1];
  pthread_mutex_unlock(&(glb->stateOut_mut));

}

// Transmissão de estado modificado
void deviceStateWrite(struct Global *glb){
  // Cópia local da estrutura stateIO
  struct StateIO stateOutLocal;

  // Copiar stateOut para estrutura temporária local
  pthread_mutex_lock(&(glb->stateOut_mut));
  memcpy(&stateOutLocal, &(glb->stateOut), sizeof(struct StateIO));
  pthread_mutex_unlock(&(glb->stateOut_mut));

  // Preparar checagem da transmissão
  stateOutLocal.serialCheck = SERIAL_TAG;

  // Enviar para o aparelho
  const char cmd = SERIAL_WRITE;
  pthread_mutex_lock(&(glb->device_mut));
  tcflush(glb->portDevice, TCIOFLUSH);
  write(glb->portDevice, &cmd, 1);
  usleep(40e+3);
  write(glb->portDevice, (char *)&stateOutLocal, sizeof(struct StateIO));
  pthread_mutex_unlock(&(glb->device_mut));
  
}

int init_tty(int port, speed_t speed) {
  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if (tcgetattr(port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in
                          // communication (most common)
  tty.c_cflag &= ~CSIZE;  // Clear all bits that set the data size
  tty.c_cflag |= CS8;     // 8 bits per byte (most common)
  tty.c_cflag &=
      ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |=
      CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;   // Disable echo
  tty.c_lflag &= ~ECHOE;  // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG;   // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                   ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g.
                         // newline chars)
  tty.c_oflag &=
      ~ONLCR; // Prevent conversion of newline to carriage return/line feed

  tty.c_cc[VTIME] = 30; // Wait for up to 3s (30 deciseconds), returning as soon
                        // as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate
  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);

  // Save tty settings, also checking for error
  if (tcsetattr(port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  return 0;
}
