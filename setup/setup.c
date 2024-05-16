#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "serial.h"
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <pthread.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>

// Estrutura com estado do aparelho
#include "../arduino/device/state.h"
struct State state;
struct StateIO stateOut;

// mutex de acesso ao estado
pthread_mutex_t state_mut = PTHREAD_MUTEX_INITIALIZER;

// Portas para comunicação serial
int portDevice;
int portProbe;

// Comunicação com GUI via socket
const char socket_path[] = "gui/socket";

// PID do processo filho assumido pela GUI
int PIDGUI;

// Intervalo entre comunicações na porta serial (microsegundos)
#define SERIAL_PAUSE 40e+3

// Temperaturas da sonda
float tempProbe[4];

// One byte commands
void deviceCmd(char cmd){
  // Flushes both data received but not read,
  // and data written but not transmitted.
  tcflush(portDevice, TCIOFLUSH);
  write(portDevice, &cmd, 1);
  usleep(SERIAL_PAUSE);
}

void probeRead(){

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

  write(portProbe, writeOut, 5);

  usleep(SERIAL_PAUSE);

  rx_bytes = read(portProbe, readIn, 13);

  // Decodificar temperaturas
  tempProbe[0] = (float)((readIn[5] << 8) | (readIn[4] & 0xff)) / 10.0;
  tempProbe[1] = (float)((readIn[7] << 8) | (readIn[6] & 0xff)) / 10.0;
  tempProbe[2] = (float)((readIn[9] << 8) | (readIn[8] & 0xff)) / 10.0;
  tempProbe[3] = (float)((readIn[11] << 8) | (readIn[10] & 0xff)) / 10.0;

  usleep(SERIAL_PAUSE);

}

// Leitura do estado no aparelho
void deviceStateRead(){

  int rx_bytes = 0;

  // Preparar checagem da transmissão
  state.serialCheck = ~ SERIAL_TAG;

  while (1){

    deviceCmd(SERIAL_READ);

    // Read the state
    rx_bytes = 0;
    for (int b = 0; b < sizeof(struct State); b += 4){
      rx_bytes += read(portDevice, (char*)&state + b, 4);
    }
    
    // Repetir se houver inconsistência no recebimento
    if ( state.serialCheck != SERIAL_TAG )
      usleep(SERIAL_PAUSE);
    else
      break;

  }

  // Leitura da sonda
  probeRead();

  // Atualizar a estrutura de envio com o estado recebido
  stateOut.tempTarget = state.tempTarget;
  stateOut.tempStep = state.tempStep;
  stateOut.fan = state.fan;
  stateOut.PID_enabled = state.PID_enabled;
  stateOut.heat = (state.PID_enabled) ? 0 : state.cPID[4];
  stateOut.cTemp[0] = state.cTemp[0];
  stateOut.cTemp[1] = state.cTemp[1];
  stateOut.cTemp[2] = state.cTemp[2];
  stateOut.cTemp[3] = state.cTemp[3];
  stateOut.cPID[0] = state.cPID[0];
  stateOut.cPID[1] = state.cPID[1];
  stateOut.cPID[2] = state.cPID[2];
  stateOut.autostop = state.autostop;
  stateOut.screensaver = state.screensaver;
  stateOut.cStop[0] = state.cStop[0];
  stateOut.cStop[1] = state.cStop[1];

}

// Transmissão de estado modificado
void deviceStateWrite(){
  // Preparar checagem da transmissão
  stateOut.serialCheck = SERIAL_TAG;

  deviceCmd(SERIAL_WRITE);

  write(portDevice, (char *)&stateOut, sizeof(struct StateIO));
  
  usleep(SERIAL_PAUSE);

  // Ler o estado no dispositivo
  //deviceStateRead();
}


// Release memory used to parse the command line
void tokens_cleanup(char **tokens) {
  for (int i = 0; tokens[i] != NULL; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

void external_cmd(char **tokens) {

  // Ignorar se comando não for "gui"
  if (strcmp("gui", tokens[0]) != 0) {
    return;
  }

  pid_t cmdpid;
  int wstatus;
  char *env[] = { NULL };

  cmdpid = fork();

  if ( cmdpid == 0 ){
    // Child process
    if (strcmp("gui", tokens[0]) == 0) {
      // Disparar node index.js
      static char *tokensGui[] = { "./gui.sh", NULL };
      execve(tokensGui[0], tokensGui, env);
    }
    else {
      // Linha do comando é tokens
      execve(tokens[0], tokens, env);
    }
    // Exit if execve fails
    exit(EXIT_FAILURE);
  }
  else {
    // Parent process
    // Retornar imediatamente se GUI
    if (strcmp("gui", tokens[0]) == 0) {
      PIDGUI = cmdpid;
      waitpid(cmdpid, &wstatus, WNOHANG | WUNTRACED | WCONTINUED);
    }
    else {
      waitpid(cmdpid, &wstatus, 0);
      if ( ! WIFEXITED(wstatus) ){
        printf("%s\n", "Error calling external command");
      }
    }
  }
}

// Parse and run cmdline. Returns
// zero value to keep the shell running
// or non-zero value to end the shell.
int exec(char *cmdline) {

  // Parse command line
  char **tokens = (char **)NULL;
  char *token = (char *)NULL;
  int t = 0;
  token = strtok(cmdline, " \t");
  while (token != NULL) {
    t++;
    tokens = (char **)realloc(tokens, t * sizeof(char *));
    tokens[t - 1] = (char *)malloc((1 + strlen(token)) * sizeof(char));
    strcpy(tokens[t - 1], token);
    token = strtok(NULL, " \t");
  }
  tokens = (char **)realloc(tokens, (1 + t) * sizeof(char *));
  tokens[t] = (char *)NULL;

  if (t == 0) {
    // Empty command line
    return 0;
  }

  /*
   * Builtin commands
   */

  // Coeficientes de temperatura pós calibragem
  if (!strcmp("ctemp", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.cTemp[0] = atof(tokens[1]);
    stateOut.cTemp[1] = atof(tokens[2]);
    stateOut.cTemp[2] = atof(tokens[3]);
    stateOut.cTemp[3] = atof(tokens[4]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s %.8f %.8f %.8f %.8f\n", tokens[0], stateOut.cTemp[0],
           stateOut.cTemp[1], stateOut.cTemp[2], stateOut.cTemp[3]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Resetar para definições padrão
  if (!strcmp("reset", tokens[0])) {

    deviceCmd(SERIAL_RESET);

    printf("%s\n", tokens[0]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Salvar definições na EEPROM
  if (!strcmp("store", tokens[0])) {

    deviceCmd(SERIAL_STORE);

    printf("%s\n", tokens[0]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Habilitar descanso de tela
  if (!strcmp("screensaver", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.screensaver = atoi(tokens[1]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.screensaver);

    tokens_cleanup(tokens);
    return 0;
  }

  // Set tempStep
  if (!strcmp("tempstep", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.tempStep = atoi(tokens[1]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.tempStep);

    tokens_cleanup(tokens);
    return 0;
  }

  // Set target
  if (!strcmp("target", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.tempTarget = atof(tokens[1]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.tempTarget);

    tokens_cleanup(tokens);
    return 0;
  }

  // Ativar
  if (!strcmp("on", tokens[0])) {

    deviceCmd(SERIAL_START);

    printf("%s\n", tokens[0]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Desativar
  if (!strcmp("off", tokens[0])) {

    deviceCmd(SERIAL_STOP);

    printf("%s\n", tokens[0]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Mostrar splash screen
  if (!strcmp("splash", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.splash =
        (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.splash);

    tokens_cleanup(tokens);
    return 0;
  }

  // Fan load
  if (!strcmp("fan", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.fan = atoi(tokens[1]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.fan);

    tokens_cleanup(tokens);
    return 0;
  }

  // PID disable/enable
  if (!strcmp("pid", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.PID_enabled =
        (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;

    // Zerar carga na resistência
    stateOut.heat = 0;

    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.PID_enabled);

    tokens_cleanup(tokens);
    return 0;
  }

  // Autostop disable/enable
  if (!strcmp("autostop", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.autostop =
        (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.autostop);

    tokens_cleanup(tokens);
    return 0;
  }

  // Limiares da parada automática
  if (!strcmp("cstop", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.cStop[0] = atof(tokens[1]);
    stateOut.cStop[1] = atof(tokens[2]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s %.8f %.8f\n", tokens[0], stateOut.cStop[0], stateOut.cStop[1]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Coeficientes de ponderação do PID
  if (!strcmp("cpid", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.cPID[0] = atof(tokens[1]);
    stateOut.cPID[1] = atof(tokens[2]);
    stateOut.cPID[2] = atof(tokens[3]);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s %.8f %.8f %.8f\n", tokens[0], stateOut.cPID[0], stateOut.cPID[1],
           stateOut.cPID[2]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Heater value (0-255)
  if (!strcmp("heat", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.heat = atoi(tokens[1]);
    stateOut.heat = (stateOut.heat < 0) ? 0 : stateOut.heat;
    stateOut.heat = (stateOut.heat > 255) ? 255 : stateOut.heat;
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.heat);

    tokens_cleanup(tokens);
    return 0;
  }

  // Fetch state
  if (!strcmp("fetch", tokens[0])) {

    pthread_mutex_lock(&state_mut);
    deviceStateRead();

    printf("elapsed           → %d\n", (int)state.elapsed);
    printf("tempCore          → %.2f\n", state.tempCore);
    printf("tempEx            → %.2f\n", state.tempEx);
    printf("tempTarget        → %.2f\n", state.tempTarget);
    printf("analogTherm       → %d\n", (int)state.analogTherm);
    printf("cTemp             → %f %f %f %f\n", state.cTemp[0], state.cTemp[1], state.cTemp[2], state.cTemp[3]);
    printf("autostop          → %d\n", (int)state.autostop);
    printf("sStop             → %f %f\n", state.sStop[0], state.sStop[1]);
    printf("cStop             → %f %f\n", state.cStop[0], state.cStop[1]);
    printf("on                → %d\n", (int)state.on);
    printf("fan               → %d\n", (int)state.fan);
    printf("cPID              → %f %f %f\n", state.cPID[0], state.cPID[1], state.cPID[2]);
    printf("PID               → %f %f %f %f %f\n", state.PID[0], state.PID[1], state.PID[2], state.PID[3], state.PID[4]);
    printf("PID_enabled       → %d\n", (int)state.PID_enabled);
    printf("ts                → %ld\n", state.ts);
    printf("tempStep          → %d\n", (int)state.tempStep);
    printf("targetLastChange  → %ld\n", state.targetLastChange);
    printf("splash            → %d\n", (int)state.splash);
    printf("screensaver       → %d\n", (int)state.screensaver);
    printf("tempProbe         → %.2f %.2f %.2f %.2f\n", tempProbe[0], tempProbe[1], tempProbe[2], tempProbe[3]);

    pthread_mutex_unlock(&state_mut);

    tokens_cleanup(tokens);
    return 0;
  }

  /*
   * External commands
   */
  external_cmd(tokens);

  tokens_cleanup(tokens);
  return 0;
}


// Comunicação via socket
void *pthread_socket(void *arg) {
  struct sockaddr_un name;
  int ret;
  int connection_socket;
  int data_socket;
  const int socket_buf_size = 512;
  char buffer[socket_buf_size];
  memset(buffer, 0, socket_buf_size);

  /*
   * In case the program exited inadvertently on the last run,
   * remove the socket.
   */

  unlink(socket_path);

  /* Create local socket. */

  connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (connection_socket == -1) {
    printf("Erro na criação do socket\n");
    exit(EXIT_FAILURE);
  }

  /*
   * For portability clear the whole structure, since some
   * implementations have additional (nonstandard) fields in
   * the structure.
   */

  memset(&name, 0, sizeof(struct sockaddr_un));

  /* Bind socket to socket name. */

  name.sun_family = AF_UNIX;
  strncpy(name.sun_path, socket_path, sizeof(name.sun_path) - 1);

  ret = bind(connection_socket, (const struct sockaddr *)&name,
             sizeof(struct sockaddr_un));
  if (ret == -1) {
    printf("Erro ao vincular o caminho do socket");
    exit(EXIT_FAILURE);
  }

  /*
   * Prepare for accepting connections. The backlog size is set
   * to 20. So while one request is being processed other requests
   * can be waiting.
   */

  ret = listen(connection_socket, 20);
  if (ret == -1) {
    printf("Erro na escuta de conexões via socket");
    exit(EXIT_FAILURE);
  }

  /* This is the main loop for handling connections. */

  while (1) {

    /* Wait for incoming connection. */

    data_socket = accept(connection_socket, NULL, NULL);
    if (data_socket == -1) {
      printf("Erro ao aceitar conexão via socket\n");
      break;
    }

    /* Wait for next data packet. */

    ret = read(data_socket, buffer, socket_buf_size);
    if (ret == -1) {
      printf("Erro no recebimento de dados via socket\n");
      break;
    }

    /* Ensure buffer is 0-terminated. */

    buffer[socket_buf_size - 1] = 0;

    /* Handle commands. */

    if (!strncmp(buffer, "END", socket_buf_size)) {
      break;
    }

    // Estado da sessão
    if (!strncmp(buffer, "STATE", socket_buf_size)) {

      pthread_mutex_lock(&state_mut);
      deviceStateRead();
      snprintf(buffer, socket_buf_size,
               "{"
               "\"elapsed\": %d,"
               "\"screensaver\": %d,"
               "\"tempStep\": %d,"
               "\"on\": %d,"
               "\"fan\": %d,"
               "\"cTemp\": [%.6f, %.6f, %.6f, %.6f],"
               "\"PID\": [%.2f, %.2f, %.2f, %.2f, %.2f],"
               "\"PID_enabled\": %d,"
               "\"cPID\": [%.8f, %.8f, %.8f],"
               "\"autostop\": %d,"
               "\"cStop\": [%.6f, %.6f],"
               "\"sStop\": [%.6f, %.6f],"
               "\"ts\": %d,"
               "\"tempCore\": %.2f,"
               "\"tempEx\": %.2f,"
               "\"tempTarget\": %.2f,"
               "\"analogTherm\": %d,"
               "\"tempProbe\": [%.2f, %.2f, %.2f, %.2f]"
               "}",
               state.elapsed, state.screensaver, state.tempStep, state.on, state.fan,
               state.cTemp[0], state.cTemp[1], state.cTemp[2], state.cTemp[3],
               state.PID[0], state.PID[1], state.PID[2], state.PID[3],
               state.PID[4], state.PID_enabled, state.cPID[0], state.cPID[1],
               state.cPID[2], state.autostop, state.cStop[0], state.cStop[1],
               state.sStop[0], state.sStop[1], state.ts,
               state.tempCore, state.tempEx, state.tempTarget, state.analogTherm,
               tempProbe[0], tempProbe[1], tempProbe[2], tempProbe[3]);
      pthread_mutex_unlock(&state_mut);
    } else {
      // Se não for solicitação do estado, executar comando
      exec(buffer);
    }

    /* Send result. */

    ret = write(data_socket, buffer, strlen(buffer));
    if (ret == -1) {
      printf("Erro no envio de resposta via socket");
      break;
    }

    memset(buffer, 0, socket_buf_size);

    /* Close socket. */

    close(data_socket);
  }

  // Execution never gets here
  close(connection_socket);

  pthread_exit((void *)NULL);
}


int main(int argc, char **argv) {

  // Portas de comunicação serial
  // TODO: Permitir escolher os caminhos
  const char pathDevice[] = "/dev/ttyUSB0";
  const char pathProbe[] = "/dev/ttyUSB1";

  portDevice = open(pathDevice, O_RDWR);
  portProbe = open(pathProbe, O_RDWR);

  int init_tty_return = init_tty(portDevice, B115200);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n",
            pathDevice, init_tty_return);
  }


  init_tty_return = init_tty(portProbe, B9600);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n",
            pathProbe, init_tty_return);
  }

  // Wait until serial port and device are ready
  sleep(3);
  deviceStateRead();

  // socket thread
  int pthread_return;
  pthread_t pthread_socket_id;
  pthread_return = pthread_create(&pthread_socket_id, NULL, &pthread_socket, (void *)NULL);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", pthread_return);
    exit(-1);
  }


  /*
   * Shell
   */

  // The username from evironment variable USER
  char *user = getenv("USER");

  // The workdir from evironment variable PWD
  // char *pwd = getenv("PWD");

  // The command prompt
  char *prompt = malloc(1024);
  // snprintf(prompt, 1024, "{%s@%s} ", user, pwd);
  snprintf(prompt, 1024, "{%s@device} ", user);

  // Command line returned by the user
  char *cmdline;

  // Stop the main loop if end != 0
  int end = 0;

  // The main loop for handling commands
  while (!end) {
    cmdline = readline(prompt);

    if (cmdline == NULL) {
      // EOF in an empty line, break prompt loop
      break;
    }

    // Add cmdline to the command history
    if (cmdline && *cmdline) {
      add_history(cmdline);
    }

    // Run the command
    end = exec(cmdline);

    // Release cmdline memory
    free(cmdline);
  }

  // End serial communication
  close(portDevice);
  close(portProbe);

  // Stop GUI
  if ( PIDGUI > 0 )
    kill(PIDGUI, SIGTERM);

  // Stop socket thread 
  pthread_kill(pthread_socket_id, SIGTERM);
  //pthread_return = pthread_join(pthread_socket_id, NULL);
  //if ( pthread_return != 0 ){
  //  fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", pthread_return);
  //}

  // Released aloccated memory
  free(prompt);

  return 0;
}
