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

#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <pthread.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>

// Estruturas com o estado do aparelho
// usadas para comunicação serial
#include "../arduino/device/state.h"

// Estrutura para acessar variáveis globais
struct Global {

  // Comunicação com GUI via socket
  char socket_path[32];

  struct State state;
  struct StateIO stateOut;

  // Temperaturas da sonda
  float tempProbe[4];

  // mutex de acesso serial ao aparelho
  pthread_mutex_t device_mut;

  // mutex de acesso ao estado do aparelho
  pthread_mutex_t state_mut;

  // mutex de acesso ao estado enviado ao aparelho
  pthread_mutex_t stateOut_mut;

  // mutex de acesso à temperatura da sonda
  pthread_mutex_t tempProbe_mut;

  // mutex de acesso serial à sonda
  pthread_mutex_t probe_mut;

  // Portas para comunicação serial
  char pathDevice[32];
  int portDevice;
  char pathProbe[32];
  int portProbe;

  // PID do processo filho assumido pela GUI
  int PIDGUI;

  // Sinalizar encerramento para threads
  int exit;
};

/*
 * Forward declarations
 */

int init_tty(int port, speed_t speed);
void deviceCmd(char cmd, struct Global *glb);
void probeRead_TA612c(struct Global *glb);
void probeRead_MAX6675(struct Global *glb);
void deviceStateRead(struct Global *glb);
void deviceStateWrite(struct Global *glb);
void tokens_cleanup(char **tokens);
void external_cmd(char **tokens, struct Global *glb);
int exec(char *cmdline, struct Global *glb);
void *pthread_updateState(void *arg);
void *pthread_updateProbe(void *arg);
void *pthread_socket(void *arg);
void *pthread_shell(void *arg);
