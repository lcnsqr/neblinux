#include "setup.h"

int main(int argc, char **argv) {

  // Estrutura para acessar variáveis globais
  struct Global glb;
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

  // Portas de comunicação serial
  snprintf(glb.pathDevice, 32, "%s", "/dev/ttyUSB0");
  //snprintf(glb.pathProbe, 32, "%s", "/dev/ttyUSB1");
  snprintf(glb.pathProbe, 32, "%s", "/dev/ttyACM0");

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
  pthread_return = pthread_create(&pthread_update_probe_id, NULL, &pthread_updateProbe, (void *)&glb);
  if (pthread_return != 0) {
    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", pthread_return);
    exit(-1);
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
  pthread_return = pthread_join(pthread_update_probe_id, NULL);
  if ( pthread_return != 0 ){
    fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", pthread_return);
  }

  return 0;
}
