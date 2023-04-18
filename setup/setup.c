#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <pthread.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>

// Estrutura com estado do aparelho
#include "../arduino/vapomatic/state.h"
struct State state;
struct StateIO stateOut;
// Variável de controle para enviar estado alterado
uint8_t state_change = 0;

// mutex de acesso ao estado
pthread_mutex_t state_mut = PTHREAD_MUTEX_INITIALIZER;

// Intervalo entre comunicações na porta serial (microsegundos)
#define RX_PAUSE 200e+3
#define TX_PAUSE 200e+3

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

// Release memory used to parse the command line
void tokens_cleanup(char **tokens) {
  for (int i = 0; tokens[i] != NULL; i++) {
    free(tokens[i]);
  }
  free(tokens);
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
    state_change = 1;
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
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s %.8f %.8f %.8f %.8f\n", tokens[0], stateOut.cTemp[0], stateOut.cTemp[1], stateOut.cTemp[2], stateOut.cTemp[3]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Salvar definições na EEPROM
  if (!strcmp("store", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.store = 1;
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s\n", tokens[0]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Set tempStep
  if (!strcmp("tempstep", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.tempStep = atoi(tokens[1]);
    state_change = 1;
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
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.tempTarget);

    tokens_cleanup(tokens);
    return 0;
  }

  // Ativar
  if (!strcmp("on", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.PID_enabled = 1;
    stateOut.on = 1;
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.on);

    tokens_cleanup(tokens);
    return 0;
  }

  // Desativar
  if (!strcmp("off", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.PID_enabled = 1;
    stateOut.on = 0;
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.on);

    tokens_cleanup(tokens);
    return 0;
  }

  // Fan control
  if (!strcmp("fan", tokens[0])) {

    // Change state
    pthread_mutex_lock(&state_mut);
    stateOut.fan =
        (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    state_change = 1;
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
    state_change = 1;
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
    state_change = 1;
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
    state_change = 1;
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
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s %.8f %.8f %.8f\n", tokens[0], stateOut.cPID[0], stateOut.cPID[1], stateOut.cPID[2]);

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
    state_change = 1;
    pthread_mutex_unlock(&state_mut);

    printf("%s = %d\n", tokens[0], (int)stateOut.heat);

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

  /*
  pid_t cmdpid;
  int wstatus;
  char *env[] = { NULL };

  cmdpid = fork();

  if ( cmdpid == 0 ){
    // Child process
    execve(tokens[0], tokens, env);
    // Exit if execve fails
    exit(EXIT_FAILURE);
  }
  else {
    // Parent process
    waitpid(cmdpid, &wstatus, 0);
    if ( ! WIFEXITED(wstatus) ){
      printf("%s\n", "Error calling external command");
    }
  }
  */

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
               state.elapsed, state.tempStep, state.on, state.fan, state.cTemp[0],
               state.cTemp[1], state.cTemp[2], state.cTemp[3], state.PID[0],
               state.PID[1], state.PID[2], state.PID[3], state.PID[4],
               state.PID_enabled, state.cPID[0], state.cPID[1], state.cPID[2],
               state.autostop, state.cStop[0], state.cStop[1], state.sStop[0], state.sStop[1],
               state.ts, graph_core, graph_probe, graph_target, graph_ex,
               graph_heat, line_core[1], line_probe[1], line_heat[1]);
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

// Comunicação com o aparelho
void *pthread_rxtx(void *arg) {

  int *port = (int *)arg;

  int rx_bytes = 0;

  int32_t header = 0x0000;

  // Simular tempos anteriores
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

  // Valores de segurança para a estrutura de controle
  stateOut.tempTarget = 180.0;
  stateOut.on = 0;
  stateOut.fan = 0;
  stateOut.PID_enabled = 1;
  stateOut.heat = 0;
  stateOut.cTemp[0] = 0;
  stateOut.cTemp[1] = 0;
  stateOut.cTemp[2] = 0;
  stateOut.cTemp[3] = 0;
  stateOut.cPID[0] = 0;
  stateOut.cPID[1] = 0;
  stateOut.cPID[2] = 0;
  stateOut.store = 0;
  stateOut.autostop = 0;

  // Substituir valores somente na primeira leitura
  int first_rx = 1;

  while (rx_bytes >= 0) {

    // Send changed state

    pthread_mutex_lock(&state_mut);

    if (state_change) {
      write(*port, (char *)&stateOut, sizeof(struct StateIO));
      state_change = 0;
      usleep(TX_PAUSE);
      // Evitar atualizar coeficientes novamente
      stateOut.cTemp[0] = 0;
      stateOut.cPID[1] = 0;
      stateOut.store = 0;
    }

    pthread_mutex_unlock(&state_mut);

    // Skip to the begining of the dataframe
    while (rx_bytes >= 0 && header != 0xffff)
      rx_bytes = read(*port, (char *)&header, 4);

    // Reset header
    header = 0x0000;

    // Read the remaining dataframe
    pthread_mutex_lock(&state_mut);
    if (!state_change) {
      for (int b = 4; b < sizeof(struct State); b += 4)
        rx_bytes = read(*port, (char *)&state + b, 4);
      // Ignorar carga de aquecimento nos primeiros milisegundos
      // Artefatos no início da comunicação serial podem aparentar picos
      if (state.ts < 5000)
        state.PID[4] = 0;
    }

    // Atualizar stateOut na primeira leitura
    if ( first_rx == 1 ) {
      stateOut.tempTarget = state.tempTarget;
      first_rx = 0;
    }

    pthread_mutex_unlock(&state_mut);

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

    usleep(RX_PAUSE);
  }

  pthread_exit((void *)NULL);
}

// Leitura da sonda
void *pthread_rx_probe(void *arg) {

  int *port = (int *)arg;

  int rx_bytes = 0;

  float rx;

  // Ganho na resistência causa um desvio na leitura da sonda
  const float drift_max = 10.0;
  float drift = 0.0;

  while (rx_bytes >= 0) {

    rx = 0;
    rx_bytes = read(*port, (char *)&rx, sizeof(float));

    if (rx_bytes != sizeof(float))
      continue;

    // Computar fator de desvio
    pthread_mutex_lock(&state_mut);
    drift = state.PID[4] / 255.0 * drift_max;
    pthread_mutex_unlock(&state_mut);

    pthread_mutex_lock(&graph_mut);
    temp_probe = rx + drift;
    pthread_mutex_unlock(&graph_mut);

    usleep(RX_PAUSE);
  }

  pthread_exit((void *)NULL);
}

int init_tty(int port_vapomatic) {
  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if (tcgetattr(port_vapomatic, &tty) != 0) {
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

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  // Save tty settings, also checking for error
  if (tcsetattr(port_vapomatic, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {

  const char serial_vapomatic[] = "/dev/ttyUSB0";
  const char serial_probe[] = "/dev/ttyACM0";

  // Resetar estado
  memset(&state, 0, sizeof(struct State));

  // Open the serial port. Change device path as needed (currently set to an
  // standard FTDI USB-UART cable type device)
  int port_vapomatic = open(serial_vapomatic, O_RDWR);
  int port_probe = open(serial_probe, O_RDWR);

  int init_tty_return = init_tty(port_vapomatic);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n",
            serial_vapomatic, init_tty_return);
    exit(-1);
  }

  init_tty_return = init_tty(port_probe);
  if (init_tty_return != 0) {
    fprintf(stderr, "ERROR; return code from init_tty() on %s is %d\n",
            serial_probe, init_tty_return);
    exit(-1);
  }

  // RXTX thread
  pthread_t pthread_rxtx_id;
  int pthread_return;

  pthread_return = pthread_create(&pthread_rxtx_id, NULL, &pthread_rxtx,
                                  (void *)&port_vapomatic);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n",
            pthread_return);
    exit(-1);
  }

  // RX probe thread
  pthread_t pthread_rx_probe_id;

  pthread_return = pthread_create(&pthread_rx_probe_id, NULL, &pthread_rx_probe,
                                  (void *)&port_probe);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n",
            pthread_return);
    exit(-1);
  }

  // socket thread
  pthread_t pthread_socket_id;
  pthread_return =
      pthread_create(&pthread_socket_id, NULL, &pthread_socket, (void *)NULL);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n",
            pthread_return);
    exit(-1);
  }

  // Wait until serial port ready
  sleep(4);

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
  snprintf(prompt, 1024, "{%s@vapomatic} ", user);

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
  close(port_vapomatic);
  pthread_return = pthread_join(pthread_rxtx_id, NULL);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_join() is %d\n",
            pthread_return);
    exit(-1);
  }

  // End serial communication with probe
  close(port_probe);
  pthread_return = pthread_join(pthread_rx_probe_id, NULL);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_join() is %d\n",
            pthread_return);
    exit(-1);
  }

  // Kill socket thread
  pthread_kill(pthread_socket_id, 15);

  // Released aloccated memory
  free(prompt);

  return 0;
}
