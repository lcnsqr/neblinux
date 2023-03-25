/*
 * getenv
 */
#include <stdlib.h>

/*
 * snprintf, readline
 */
#include <stdio.h>

/*
 * readline, add_history
 */
#include <readline/history.h>
#include <readline/readline.h>

/*
 * readline, add_history, strcpy, strtok
 */
#include <string.h>

/*
 * mkdir
 */
#include <sys/stat.h>

/*
 * kill
 */
#include <signal.h>

/*
 * symlink
 */
#include <unistd.h>

/*
 * fork, execve
 */
#include <unistd.h>

/*
 * fork, mkdir, kill
 */
#include <sys/types.h>

/*
 * wait
 */
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>   // Error integer and strerror() function
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions

#include <pthread.h>
#include <stdlib.h>

// Estrutura com estado do aparelho
#include "../arduino/vapomatic/state.h"
struct State state;
struct StateIO stateOut;
// Variável de controle para enviar estado alterado
uint8_t state_change = 0;
// mutex de alteração do estado
pthread_mutex_t state_mut = PTHREAD_MUTEX_INITIALIZER;

// Operações matemáticas
#include "mat.h"

#define GRAPH_POINTS 120
#define GRAPH_STEP 0.25

// mutex do histórico de temperaturas
pthread_mutex_t temps_mut = PTHREAD_MUTEX_INITIALIZER;

// Temperature sequences for graphs
struct {
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
} temps;

// Avaliar comportamento da temperatura
void *pthread_temp_gist(void *arg) {

  // Create gnuplot process to pipe in temp linear regression
  FILE *gnuplot = popen("gnuplot 2>/dev/null", "w");

  // Mostrar coeficientes no fd 3
  FILE *fd_coefs = fdopen(3, "w");

  static const int tail_points = 24;
  static const float y_temp_max = 400.0;
  static const float y_heat_max = 255.0;

  // Nos testes, o slope estável será < 1e-3 = 0.001 no domínio e imagem
  // normalizadis

  float tail_x[tail_points];

  float tail_core[tail_points];
  float tail_core_line[tail_points];

  float tail_probe[tail_points];
  float tail_probe_line[tail_points];

  float tail_heat[tail_points];
  float tail_heat_line[tail_points];

  for (int i = 0; i < tail_points; i++)
    tail_x[i] = (float)i / (float)tail_points;

  float coefs_core[2];
  float coefs_probe[2];
  float coefs_heat[2];

  fprintf(gnuplot, "set xrange [%.2f:%.2f]\n", 0.0, 1.0);
  fprintf(gnuplot, "set yrange [%.2f:%.2f]\n", -1.0, 1.0);
  fprintf(gnuplot, "set title font \",14\"\n");
  fprintf(gnuplot, "set title \"RL últimos %d pontos\"\n", tail_points);

  while (fflush(gnuplot) == 0) {

    sleep(GRAPH_STEP);

    fprintf(gnuplot, "%s\n",
            "plot \"-\" using 1:2 title \"Interna\" with lines lw 2, \"-\" "
            "using 1:3 title \"Sonda\" with lines lw 2, \"-\" using 1:4 title "
            "\"Aquecimento\" with lines lw 2");

    pthread_mutex_lock(&temps_mut);

    for (int j = 0; j < tail_points; j++) {
      tail_core[j] =
          temps.core[(temps.i_core + (GRAPH_POINTS - tail_points) + j) %
                     GRAPH_POINTS] /
          y_temp_max;
      tail_probe[j] =
          temps.probe[(temps.i_probe + (GRAPH_POINTS - tail_points) + j) %
                      GRAPH_POINTS] /
          y_temp_max;
      tail_heat[j] =
          temps.heat[(temps.i_heat + (GRAPH_POINTS - tail_points) + j) %
                     GRAPH_POINTS] /
          y_heat_max;
    }

    pthread_mutex_unlock(&temps_mut);

    // Regressão linear core
    mat_leastsquares(tail_points, 1, tail_x, tail_core, coefs_core);

    // Regressão linear sonda
    mat_leastsquares(tail_points, 1, tail_x, tail_probe, coefs_probe);

    // Regressão linear aquecimento
    mat_leastsquares(tail_points, 1, tail_x, tail_heat, coefs_heat);

    // fprintf(fd_coefs, "%5.6f\t%5.6f\t%5.6f\t%5.6f\n", coefs_core[0],
    // coefs_core[1], coefs_probe[0], coefs_probe[1]);
    fprintf(fd_coefs, "%5.6f\t%5.6f\n", coefs_heat[0], coefs_heat[1]);

    // Retas core e sonda e aquecimento
    for (int j = 0; j < tail_points; j++) {
      tail_core_line[j] = coefs_core[0] + coefs_core[1] * tail_x[j];
      tail_probe_line[j] = coefs_probe[0] + coefs_probe[1] * tail_x[j];
      tail_heat_line[j] = coefs_heat[0] + coefs_heat[1] * tail_x[j];
      fprintf(gnuplot, "%.2f\t%.2f\t%.2f\t%.2f\n", tail_x[j], tail_core_line[j],
              tail_probe_line[j], tail_heat_line[j]);
    }

    fprintf(gnuplot, "%s\n", "e");
  }

  pthread_exit((void *)NULL);
}

// Plotagem
void *pthread_plot(void *arg) {

  FILE *gnuplot = popen("gnuplot 2>/dev/null", "w");

  // fprintf(gnuplot, "set multiplot layout 1, 2\n");

  fprintf(gnuplot, "set xrange [%.2f:%.2f]\n",
          -(float)GRAPH_POINTS * GRAPH_STEP, -(float)GRAPH_STEP);
  fprintf(gnuplot, "set yrange [0:400]\n");
  fprintf(gnuplot, "set title font \",14\"\n");
  fprintf(gnuplot, "set title \"Temperaturas\"\n");

  while (fflush(gnuplot) == 0) {

    sleep(GRAPH_STEP);

    fprintf(
        gnuplot, "%s\n",
        "plot \"-\" using 1:2 title \"Interna\" with lines lw 2, \"-\" using "
        "1:3 title \"Sonda\" with lines lw 2, \"-\" using 1:4 title \"Alvo\" "
        "with lines lw 2, \"-\" using 1:5 title \"Saída\" with lines lw 2");

    pthread_mutex_lock(&temps_mut);

    for (int j = 0; j < GRAPH_POINTS; j++) {
      fprintf(gnuplot, "%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",
              -(float)GRAPH_POINTS * GRAPH_STEP + (float)j * GRAPH_STEP,
              temps.core[(temps.i_core + j) % GRAPH_POINTS],
              temps.probe[(temps.i_probe + j) % GRAPH_POINTS],
              temps.target[(temps.i_target + j) % GRAPH_POINTS],
              temps.ex[(temps.i_ex + j) % GRAPH_POINTS]);
    }

    pthread_mutex_unlock(&temps_mut);

    fprintf(gnuplot, "%s\n", "e");
  }

  pthread_exit((void *)NULL);
}

// Comunicação com o aparelho
void *pthread_rxtx(void *arg) {

  int *port = (int *)arg;

  int rx_bytes = 0;

  int32_t header = 0x0000;

  memset(temps.core, 0, GRAPH_POINTS * sizeof(float));
  memset(temps.heat, 0, GRAPH_POINTS * sizeof(float));

  temps.i_core = 0;
  temps.i_ex = 0;
  temps.i_target = 0;
  temps.i_heat = 0;

  while (rx_bytes >= 0) {

    // Send changed state

    pthread_mutex_lock(&state_mut);

    if (state_change) {
      write(*port, (char *)&stateOut, sizeof(struct StateIO));
      state_change = 0;
      sleep(0.1);
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
    }

    // Atualizar stateOut
    /*
    stateOut.tempTarget = state.tempTarget;
    stateOut.on = state.on;
    stateOut.fan = state.fan;
    stateOut.PID_enabled = (uint32_t)state.PID[5];
    stateOut.heat = (uint32_t)state.PID[4];
    */

    pthread_mutex_unlock(&state_mut);

    pthread_mutex_lock(&temps_mut);

    temps.core[temps.i_core] = state.tempCore;
    temps.i_core = (temps.i_core + 1) % GRAPH_POINTS;

    temps.ex[temps.i_ex] = state.tempEx;
    temps.i_ex = (temps.i_ex + 1) % GRAPH_POINTS;

    temps.target[temps.i_target] = state.tempTarget;
    temps.i_target = (temps.i_target + 1) % GRAPH_POINTS;

    temps.heat[temps.i_heat] = state.PID[4];
    temps.i_heat = (temps.i_heat + 1) % GRAPH_POINTS;

    pthread_mutex_unlock(&temps_mut);
  }

  pthread_exit((void *)NULL);
}

// Leitura da sonda
void *pthread_rx_probe(void *arg) {

  int *port = (int *)arg;

  int rx_bytes = 0;

  float rx;
  memset(temps.probe, 0, GRAPH_POINTS * sizeof(float));
  temps.i_probe = 0;

  while (rx_bytes >= 0) {

    rx = 0;
    rx_bytes = read(*port, (char *)&rx, sizeof(float));

    if (rx_bytes != sizeof(float))
      continue;

    pthread_mutex_lock(&temps_mut);

    temps.probe[temps.i_probe] = rx;
    temps.i_probe = (temps.i_probe + 1) % GRAPH_POINTS;

    pthread_mutex_unlock(&temps_mut);
  }

  pthread_exit((void *)NULL);
}

/*
 * Forward declarations
 */

// Parse and run cmdline. Returns
// zero value to keep the shell running
// or non-zero value to end the shell.
int exec(char *cmdline);

// Release memory used to parse the command line
void tokens_cleanup(char **tokens);

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

  // Set in/out baud rate to be 115200
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

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

  // Valores padrão para a estrutura de controle
  stateOut.tempTarget = 100.0;
  stateOut.on = 0;
  stateOut.fan = 0;
  stateOut.PID_enabled = 1;
  stateOut.heat = 0;

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

  // Plot thread
  pthread_t pthread_plot_id;

  pthread_return =
      pthread_create(&pthread_plot_id, NULL, &pthread_plot, (void *)NULL);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n",
            pthread_return);
    exit(-1);
  }

  // Plot linear regression
  pthread_t pthread_temp_gist_id;

  pthread_return = pthread_create(&pthread_temp_gist_id, NULL,
                                  &pthread_temp_gist, (void *)NULL);
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

  // Close gnuplot
  pthread_kill(pthread_plot_id, 15);
  pthread_kill(pthread_temp_gist_id, 15);

  // End serial communication with ship
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

  // Released aloccated memory
  free(prompt);

  return 0;
}

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

void tokens_cleanup(char **tokens) {
  for (int i = 0; tokens[i] != NULL; i++) {
    free(tokens[i]);
  }
  free(tokens);
}
