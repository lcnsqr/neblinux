#include "setup.h"
#include <getopt.h>

int main(int argc, char **argv) {

  // Estrutura para acessar variáveis globais
  struct Global glb;

  // Valor padrão de parâmetro
  snprintf(glb.pathDevice, 32, "%s", "/dev/ttyUSB0");
  snprintf(glb.pathProbe, 32, "%s", "/dev/ttyACM0");
  glb.probe = 0;

  // Tratar parâmetros informados
  int c;

  while (1) {
    static struct option long_param[] = {
      {"devicepath",  required_argument, 0, 'd'},
      {"probepath",   required_argument, 0, 'p'},
      {"probe",   required_argument, 0, 'm'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    c = getopt_long(argc, argv, "d:p:m:", long_param, &option_index);
    if (c == -1) break;

    switch (c) {

      case 'd':
        snprintf(glb.pathDevice, 32, "%s", optarg);
        break;

      case 'p':
        snprintf(glb.pathProbe, 32, "%s", optarg);
        break;

      case 'm':
        if ( ! strcmp("none", optarg) )
          glb.probe = 0;
        else if ( ! strcmp("uno", optarg) )
          glb.probe = 1;
        else if ( ! strcmp("ta612c", optarg) )
          glb.probe = 2;
        else
          glb.probe = 0;
        break;

      case '?':
      default:
        fprintf(stderr, "Usage: %s [OPTION]\n", argv[0]);
        fprintf(stderr, "  -d, --devicepath    Caminho para a porta serial do aparelho\n");
        fprintf(stderr, "  -p, --probepath     Caminho para a porta serial da sonda\n");
        fprintf(stderr, "  -m, --probe         Sonda utilizada: none, uno ou ta612c\n");
        exit(EXIT_FAILURE);
        break;

    }
  }

  // Limpar dados nas estruturas de estado
  memset(&(glb.state), 0, sizeof(struct State));
  memset(&(glb.stateOut), 0, sizeof(struct StateIO));

  // Comunicação com GUI via socket
  snprintf(glb.socket_path, 32, "%s", "gui/socket");

  pthread_mutex_init(&(glb.device_mut), NULL);
  pthread_mutex_init(&(glb.state_mut), NULL);
  pthread_mutex_init(&(glb.stateOut_mut), NULL);
  pthread_mutex_init(&(glb.tempProbe_mut), NULL);
  pthread_mutex_init(&(glb.probe_mut), NULL);

  // Continuar laços até sinalização de encerramento
  glb.exit = 0;

  // update device state thread
  int pthread_return;
  pthread_t pthread_update_id;
  pthread_return = pthread_create(&pthread_update_id, NULL, &pthread_updateState, (void *)&glb);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", pthread_return);
    exit(-1);
  }

  // update probe thread
  pthread_t pthread_update_probe_id;
  if ( glb.probe != 0 ){
    pthread_return = pthread_create(&pthread_update_probe_id, NULL, &pthread_updateProbe, (void *)&glb);
    if (pthread_return != 0) {
      fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", pthread_return);
      exit(-1);
    }
  }

  // sockets threads
  pthread_t pthread_socket_id;
  pthread_return = pthread_create(&pthread_socket_id, NULL, &pthread_socket, (void *)&glb);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", pthread_return);
    exit(-1);
  }

  // Shell interface
  pthread_t pthread_shell_id;
  pthread_return = pthread_create(&pthread_shell_id, NULL, &pthread_shell, (void *)&glb);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", pthread_return);
    exit(-1);
  }

  // Join shell thread
  pthread_return = pthread_join(pthread_shell_id, NULL);
  if ( pthread_return != 0 ){
    fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", pthread_return);
  }

  // Stop GUI server
  if ( glb.PIDGUI > 0 )
    kill(glb.PIDGUI, SIGTERM);

  // Stop socket thread 
  pthread_kill(pthread_socket_id, SIGTERM);

  // Join update state thread
  pthread_return = pthread_join(pthread_update_id, NULL);
  if ( pthread_return != 0 ){
    fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", pthread_return);
  }

  // Join probe thread
  if ( glb.probe != 0 ){
    pthread_return = pthread_join(pthread_update_probe_id, NULL);
    if ( pthread_return != 0 ){
      fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", pthread_return);
    }
  }

  return 0;
}
