#include "setup.h"

void *pthread_socket_status(void *arg) {
  
  struct Global *glb = (struct Global*)arg;

  struct sockaddr_un name;
  int ret;
  int connection_socket;
  int data_socket;
  const int socket_buf_size = 1024;
  char buffer[socket_buf_size];
  memset(buffer, 0, socket_buf_size);


  // Cópias locais temporárias
  struct State stateLocal;
  float tempProbeLocal[4];

  /*
   * In case the program exited inadvertently on the last run,
   * remove the socket.
   */

  unlink(glb->socket_path_status);

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
  strncpy(name.sun_path, glb->socket_path_status, sizeof(name.sun_path) - 1);

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

    // Estado da sessão
    if (!strncmp(buffer, "STATE", socket_buf_size)) {

      pthread_mutex_lock(&(glb->state_mut));
      memcpy(&stateLocal, &(glb->state), sizeof(struct State));
      pthread_mutex_unlock(&(glb->state_mut));
      pthread_mutex_lock(&(glb->tempProbe_mut));
      memcpy(tempProbeLocal, glb->tempProbe, 4 * sizeof(float));
      pthread_mutex_unlock(&(glb->tempProbe_mut));

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
               stateLocal.elapsed, stateLocal.screensaver, stateLocal.tempStep, stateLocal.on, stateLocal.fan,
               stateLocal.cTemp[0], stateLocal.cTemp[1], stateLocal.cTemp[2], stateLocal.cTemp[3],
               stateLocal.PID[0], stateLocal.PID[1], stateLocal.PID[2], stateLocal.PID[3],
               stateLocal.PID[4], stateLocal.PID_enabled, stateLocal.cPID[0], stateLocal.cPID[1],
               stateLocal.cPID[2], stateLocal.autostop, stateLocal.cStop[0], stateLocal.cStop[1],
               stateLocal.sStop[0], stateLocal.sStop[1], stateLocal.ts,
               stateLocal.tempCore, stateLocal.tempEx, stateLocal.tempTarget, (int)stateLocal.analogTherm,
               tempProbeLocal[0], tempProbeLocal[1], tempProbeLocal[2], tempProbeLocal[3]);
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

void *pthread_socket_cmd(void *arg) {
  
  struct Global *glb = (struct Global*)arg;

  struct sockaddr_un name;
  int ret;
  int connection_socket;
  int data_socket;
  const int socket_buf_size = 1024;
  char buffer[socket_buf_size];
  memset(buffer, 0, socket_buf_size);


  // Cópias locais temporárias
  struct State stateLocal;
  float tempProbeLocal[4];

  /*
   * In case the program exited inadvertently on the last run,
   * remove the socket.
   */

  unlink(glb->socket_path_cmd);

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
  strncpy(name.sun_path, glb->socket_path_cmd, sizeof(name.sun_path) - 1);

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

    exec(buffer, glb);

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
