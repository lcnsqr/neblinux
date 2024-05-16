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
int port_device;
int port_probe;

// Intervalo entre comunicações na porta serial (microsegundos)
#define SERIAL_PAUSE 40e+3

// Operações matemáticas
#include "mat.h"

// Número de pontos nos gráficos
#define GRAPH_POINTS 200

// mutex do histórico de temperaturas
pthread_mutex_t graph_mut = PTHREAD_MUTEX_INITIALIZER;

// Última temperatura da sonda
float temp_probe;

// Sequences for graphs
struct {
  float ts[GRAPH_POINTS];
  int i_ts;
  float core[GRAPH_POINTS];
  int i_core;
  float ex[GRAPH_POINTS];
  int i_ex;
  float target[GRAPH_POINTS];
  int i_target;
  float probe[GRAPH_POINTS];
  int i_probe;
  float heat[GRAPH_POINTS];
  int i_heat;
} graph;

// Pontos de calibragem
#define CALIB_POINTS 9
float calibCore[CALIB_POINTS];
float calibProbe[CALIB_POINTS];
// Coeficientes do polinômio interpolador de grau 3
#define CALIB_COEFS 4

// Regressão linear nos 20 pontos recentes
#define TAIL_POINTS 20

// Leitura do estado no aparelho
void deviceStateRead(){
  // Comando de leitura
  char cmd;
  cmd = SERIAL_READ;

  int rx_bytes = 0;

  if (port_device >= 0){

    // Preparar checagem da transmissão
    state.serialCheck = ~ SERIAL_TAG;

    while (1){
      // Flushes both data received but not read,
      // and data written but not transmitted.
      tcflush(port_device, TCIOFLUSH);

      write(port_device, &cmd, 1);

      // Read the state
      rx_bytes = 0;
      for (int b = 0; b < sizeof(struct State); b += 4){
        rx_bytes += read(port_device, (char*)&state + b, 4);
      }
      
      // Repetir se houver inconsistência no recebimento
      if ( state.serialCheck != SERIAL_TAG )
        usleep(SERIAL_PAUSE);
      else
        break;

    }

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

    /*
    // Atualizar sequências do gráfico
    pthread_mutex_lock(&graph_mut);

    graph.ts[graph.i_ts] = (float)state.ts;
    graph.i_ts = (graph.i_ts + 1) % GRAPH_POINTS;

    graph.core[graph.i_core] = state.tempCore;
    graph.i_core = (graph.i_core + 1) % GRAPH_POINTS;

    graph.ex[graph.i_ex] = state.tempEx;
    graph.i_ex = (graph.i_ex + 1) % GRAPH_POINTS;

    graph.target[graph.i_target] = state.tempTarget;
    graph.i_target = (graph.i_target + 1) % GRAPH_POINTS;

    graph.heat[graph.i_heat] = state.PID[4];
    graph.i_heat = (graph.i_heat + 1) % GRAPH_POINTS;

    graph.probe[graph.i_probe] = temp_probe;
    graph.i_probe = (graph.i_probe + 1) % GRAPH_POINTS;

    pthread_mutex_unlock(&graph_mut);
    */
  }
}

// Transmissão de estado modificado
void deviceStateWrite(){

  // Comando de escrita
  char cmd;
  cmd = SERIAL_WRITE;

  if (port_device >= 0){

    // Preparar checagem da transmissão
    stateOut.serialCheck = SERIAL_TAG;

    // Flushes both data received but not read,
    // and data written but not transmitted.
    tcflush(port_device, TCIOFLUSH);

    write(port_device, &cmd, 1);
    usleep(SERIAL_PAUSE);

    write(port_device, (char *)&stateOut, sizeof(struct StateIO));
    
    usleep(SERIAL_PAUSE);

    // Ler o estado no dispositivo
    //deviceStateRead();
  }
}

void deviceCmd(char cmd){
  if (port_device >= 0){
    tcflush(port_device, TCIOFLUSH);
    write(port_device, &cmd, 1);
    usleep(SERIAL_PAUSE);
  }
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

  // Calibragem: Temperaturas internas e da sonda
  if (!strcmp("calib", tokens[0])) {

    // Ponto fixo
    calibCore[0] = 25.0;
    calibProbe[0] = 25.0;
    for (int i = 1; i < CALIB_POINTS; i++) {
      calibCore[i] = atof(tokens[i]);
      calibProbe[i] = atof(tokens[i + CALIB_POINTS - 1]);
    }

    // Change state
    pthread_mutex_lock(&state_mut);
    mat_leastsquares(CALIB_POINTS, CALIB_COEFS - 1, calibCore, calibProbe,
                     stateOut.cTemp);
    deviceStateWrite();
    pthread_mutex_unlock(&state_mut);

    printf("\n");
    for (int i = 0; i < CALIB_POINTS; i++)
      printf("%f → %f\n", calibCore[i], calibProbe[i]);

    printf("Coeficientes\n");
    for (int i = 0; i < CALIB_COEFS; i++) {
      printf("%f\n", stateOut.cTemp[i]);
    }

    printf("Função em x\n");
    float r = 0;
    for (int j = 0; j < CALIB_POINTS; j++) {
      for (int i = 0; i < CALIB_COEFS; i++) {
        r += stateOut.cTemp[i] * pow(calibCore[j], i);
      }
      printf("%f %f\n", calibCore[j], r);
      r = 0;
    }

    tokens_cleanup(tokens);
    return 0;
  }

  // Coeficientes de temperatura direto sem calibrar
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

  // Fan control
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
    pthread_mutex_unlock(&state_mut);

    printf("elapsed           → %d\n", (int)state.elapsed);
    printf("tempCore          → %d\n", (int)state.tempCore);
    printf("tempEx            → %d\n", (int)state.tempEx);
    printf("tempTarget        → %d\n", (int)state.tempTarget);
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

    tokens_cleanup(tokens);
    return 0;
  }

  /*
  // exit
  if ( ! strcmp("exit", tokens[0]) ){
    printf("%s\n", cmdline);
    tokens_cleanup(tokens);
    return 1;
  }

  // mkdir
  if ( ! strcmp("mkdir", tokens[0]) ){
    // Using 0777 as the directory permission (it will be combined with system's
  umask) mkdir(tokens[1], 0777); tokens_cleanup(tokens); return 0;
  }

  // kill
  if ( ! strcmp("kill", tokens[0]) ){
    // Signal and PID numerical values (without the hifen)
    kill(atoi(tokens[2]), atoi(&tokens[1][1]));
    tokens_cleanup(tokens);
    return 0;
  }

  // ln
  if ( ! strcmp("ln", tokens[0]) ){
    // symlink creates a symbolic link (as `ln -s` does)
    symlink(tokens[2], tokens[3]);
    tokens_cleanup(tokens);
    return 0;
  }
  */

  /*
   * External commands
   */
  external_cmd(tokens);

  tokens_cleanup(tokens);
  return 0;
}

// Computar regressão linear nos pontos recentes
void regression(float line_core[], float line_probe[], float line_heat[]) {

  float tail_x[TAIL_POINTS];
  float tail_core[TAIL_POINTS];
  float tail_probe[TAIL_POINTS];
  float tail_heat[TAIL_POINTS];

  for (int i = 0; i < TAIL_POINTS; i++)
    tail_x[i] = (float)i / (float)TAIL_POINTS;

  pthread_mutex_lock(&graph_mut);

  for (int j = 0; j < TAIL_POINTS; j++) {
    tail_core[j] =
        graph.core[(graph.i_core + (GRAPH_POINTS - TAIL_POINTS) + j) %
                   GRAPH_POINTS] /
        TEMP_MAX;
    tail_probe[j] =
        graph.probe[(graph.i_probe + (GRAPH_POINTS - TAIL_POINTS) + j) %
                    GRAPH_POINTS] /
        TEMP_MAX;
    tail_heat[j] =
        graph.heat[(graph.i_heat + (GRAPH_POINTS - TAIL_POINTS) + j) %
                   GRAPH_POINTS] /
        HEAT_MAX;
  }

  pthread_mutex_unlock(&graph_mut);

  // Regressão linear core
  mat_leastsquares(TAIL_POINTS, 1, tail_x, tail_core, line_core);

  // Regressão linear sonda
  mat_leastsquares(TAIL_POINTS, 1, tail_x, tail_probe, line_probe);

  // Regressão linear aquecimento
  mat_leastsquares(TAIL_POINTS, 1, tail_x, tail_heat, line_heat);
}

// Comunicação via socket
void *pthread_socket(void *arg) {
  struct sockaddr_un name;
  int ret;
  int connection_socket;
  int data_socket;
  const int socket_buf_size = 65536;
  char buffer[socket_buf_size];
  memset(buffer, 0, socket_buf_size);
  const char socket_path[] = "/tmp/vapomatic.sock";

  // Sequências de valores em texto para o formato json
  char graph_core[16384];
  memset(graph_core, 0, 16384);
  char graph_probe[16384];
  memset(graph_probe, 0, 16384);
  char graph_target[16384];
  memset(graph_target, 0, 16384);
  char graph_ex[16384];
  memset(graph_ex, 0, 16384);
  char graph_heat[16384];
  memset(graph_heat, 0, 16384);

  // Coeficientes de estabilidade
  float line_core[2];
  float line_probe[2];
  float line_heat[2];

  /*
   * In case the program exited inadvertently on the last run,
   * remove the socket.
   */

  unlink(socket_path);

  /* Create local socket. */

  connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (connection_socket == -1) {
    perror("socket");
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
    perror("bind");
    exit(EXIT_FAILURE);
  }

  /*
   * Prepare for accepting connections. The backlog size is set
   * to 20. So while one request is being processed other requests
   * can be waiting.
   */

  ret = listen(connection_socket, 20);
  if (ret == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  /* This is the main loop for handling connections. */

  for (;;) {

    /* Wait for incoming connection. */

    data_socket = accept(connection_socket, NULL, NULL);
    if (data_socket == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    /* Wait for next data packet. */

    ret = read(data_socket, buffer, socket_buf_size);
    if (ret == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /* Ensure buffer is 0-terminated. */

    buffer[socket_buf_size - 1] = 0;

    /* Handle commands. */

    if (!strncmp(buffer, "END", socket_buf_size)) {
      break;
    }

    // Estado da sessão
    if (!strncmp(buffer, "STATE", socket_buf_size)) {

      // Eixo x em segundos (negativos) contados pelo timestamp
      float x = 0;
      float y = 0;

      // Gerar sequências em texto
      pthread_mutex_lock(&graph_mut);
      for (int j = 0; j < GRAPH_POINTS; j++) {

        x = graph.ts[(graph.i_ts + j) % GRAPH_POINTS] -
            graph.ts[(graph.i_ts + GRAPH_POINTS - 1) % GRAPH_POINTS];
        x *= 1e-3;

        // Tratar erros no cálculo do tempo
        if (x < -60.0 || x > 0)
          x = -60.0 + (float)j * 0.5;

        y = graph.core[(graph.i_core + j) % GRAPH_POINTS];
        if (y < -60.0 || y > 1000.0)
          y = 0;
        sprintf(graph_core + j * 34, "{\"x\":\"%10.4f\",\"y\":%10.4f}%s", x, y,
                (j == GRAPH_POINTS - 1) ? " " : ",");

        y = graph.probe[(graph.i_probe + j) % GRAPH_POINTS];
        if (y < -60.0 || y > 1000.0)
          y = 0;
        sprintf(graph_probe + j * 34, "{\"x\":\"%10.4f\",\"y\":%10.4f}%s", x, y,
                (j == GRAPH_POINTS - 1) ? " " : ",");

        y = graph.target[(graph.i_target + j) % GRAPH_POINTS];
        if (y < -60.0 || y > 1000.0)
          y = 0;
        sprintf(graph_target + j * 34, "{\"x\":\"%10.4f\",\"y\":%10.4f}%s", x,
                y, (j == GRAPH_POINTS - 1) ? " " : ",");

        y = graph.ex[(graph.i_ex + j) % GRAPH_POINTS];
        if (y < -60.0 || y > 1000.0)
          y = 0;
        sprintf(graph_ex + j * 34, "{\"x\":\"%10.4f\",\"y\":%10.4f}%s", x, y,
                (j == GRAPH_POINTS - 1) ? " " : ",");

        y = graph.heat[(graph.i_heat + j) % GRAPH_POINTS];
        if (y < -60.0 || y > 1000.0)
          y = 0;
        sprintf(graph_heat + j * 34, "{\"x\":\"%10.4f\",\"y\":%10.4f}%s", x, y,
                (j == GRAPH_POINTS - 1) ? " " : ",");
      }
      pthread_mutex_unlock(&graph_mut);

      regression(line_core, line_probe, line_heat);

      pthread_mutex_lock(&state_mut);
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
               "\"graph\":"
               "{"
               "\"core\":[%s],"
               "\"probe\":[%s],"
               "\"target\":[%s],"
               "\"ex\":[%s],"
               "\"heat\":[%s]"
               "},"
               "\"deriv\":"
               "{"
               "\"core\":%.6f,"
               "\"probe\":%.6f,"
               "\"heat\":%.6f"
               "}"
               "}",
               state.elapsed, state.screensaver, state.tempStep, state.on, state.fan,
               state.cTemp[0], state.cTemp[1], state.cTemp[2], state.cTemp[3],
               state.PID[0], state.PID[1], state.PID[2], state.PID[3],
               state.PID[4], state.PID_enabled, state.cPID[0], state.cPID[1],
               state.cPID[2], state.autostop, state.cStop[0], state.cStop[1],
               state.sStop[0], state.sStop[1], state.ts, graph_core,
               graph_probe, graph_target, graph_ex, graph_heat, line_core[1],
               line_probe[1], line_heat[1]);
      pthread_mutex_unlock(&state_mut);
    } else {
      // Se não for solicitação do estado, executar comando
      exec(buffer);
    }

    /* Send result. */

    ret = write(data_socket, buffer, strlen(buffer));
    if (ret == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

    memset(buffer, 0, socket_buf_size);

    /* Close socket. */

    close(data_socket);
  }

  close(connection_socket);

  /* Unlink the socket. */

  unlink(socket_path);

  pthread_exit((void *)NULL);
}


int main(int argc, char **argv) {

  // Resetar estado
  memset(&state, 0, sizeof(struct State));

  // Simular tempos anteriores na sequências dos gráficos
  for (int i = 0; i < GRAPH_POINTS; i++)
    graph.ts[i] = -60 * 1e+3 + (float)i * 60 * 1e+3 / (float)GRAPH_POINTS;

  memset(graph.core, 0, GRAPH_POINTS * sizeof(float));
  memset(graph.ex, 0, GRAPH_POINTS * sizeof(float));
  memset(graph.target, 0, GRAPH_POINTS * sizeof(float));
  memset(graph.heat, 0, GRAPH_POINTS * sizeof(float));
  memset(graph.probe, 0, GRAPH_POINTS * sizeof(float));

  graph.i_ts = 0;
  graph.i_core = 0;
  graph.i_ex = 0;
  graph.i_target = 0;
  graph.i_heat = 0;
  graph.i_probe = 0;

  // Portas de comunicação serial
  const char serial_vapomatic[] = "/dev/ttyUSB0";
  const char serial_probe[] = "/dev/ttyACM0";

  port_device = open(serial_vapomatic, O_RDWR);
  //port_probe = open(serial_probe, O_RDWR);

  int init_tty_return = init_tty(port_device, B115200);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n",
            serial_vapomatic, init_tty_return);
  }


  /*
  init_tty_return = init_tty(port_probe, B115200);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n",
            serial_probe, init_tty_return);
  }
  */

  // Wait until serial port and device are ready
  sleep(3);
  deviceStateRead();

  // socket thread
  int pthread_return;
  pthread_t pthread_socket_id;
  pthread_return =
      pthread_create(&pthread_socket_id, NULL, &pthread_socket, (void *)NULL);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n",
            pthread_return);
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
      printf("exit\n");
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

  // End serial communication with device
  close(port_device);

  // Kill socket thread
  pthread_kill(pthread_socket_id, 15);

  // Released aloccated memory
  free(prompt);

  return 0;
}
