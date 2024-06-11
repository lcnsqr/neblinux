#include "setup.h"

void *pthread_shell(void *arg) {

  struct Global *glb = (struct Global*)arg;

  // Wait until serial port and device are ready
  sleep(3);

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

  // The main loop for handling commands
  while (! glb->exit) {
    cmdline = readline(prompt);

    if (cmdline == NULL) {
      // EOF in an empty line, break prompt loop
      glb->exit = 1;
      break;
    }

    // Add cmdline to the command history
    if (cmdline && *cmdline) {
      add_history(cmdline);
    }

    // Run the command
    glb->exit = exec(cmdline, glb);

    // Release cmdline memory
    free(cmdline);
  }

  // Released aloccated memory
  free(prompt);

  pthread_exit((void *)NULL);
}

// Release memory used to parse the command line
void tokens_cleanup(char **tokens) {
  for (int i = 0; tokens[i] != NULL; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

void external_cmd(char **tokens, struct Global *glb) {

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
      glb->PIDGUI = cmdpid;
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
int exec(char *cmdline, struct Global *glb) {

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

  // Cópias locais temporárias
  struct State stateLocal;
  float tempProbeLocal[4];

  // Coeficientes de temperatura pós calibragem
  if (!strcmp("ctemp", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.cTemp[0] = atof(tokens[1]);
    glb->stateOut.cTemp[1] = atof(tokens[2]);
    glb->stateOut.cTemp[2] = atof(tokens[3]);
    glb->stateOut.cTemp[3] = atof(tokens[4]);
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Resetar para definições padrão
  if (!strcmp("reset", tokens[0])) {

    deviceCmd(SERIAL_RESET, glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Salvar definições na EEPROM
  if (!strcmp("store", tokens[0])) {

    deviceCmd(SERIAL_STORE, glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Habilitar descanso de tela
  if (!strcmp("screensaver", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.screensaver = (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Set tempStep
  if (!strcmp("tempstep", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.tempStep = atoi(tokens[1]);
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Set target
  if (!strcmp("target", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.tempTarget = atof(tokens[1]);
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Ativar
  if (!strcmp("on", tokens[0])) {

    deviceCmd(SERIAL_START, glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Desativar
  if (!strcmp("off", tokens[0])) {

    deviceCmd(SERIAL_STOP, glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Mostrar splash screen
  if (!strcmp("splash", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.splash = (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Fan load
  if (!strcmp("fan", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.fan = atoi(tokens[1]);
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // PID disable/enable
  if (!strcmp("pid", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.PID_enabled = (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    // Zerar carga na resistência
    glb->stateOut.heat = 0;
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Autostop disable/enable
  if (!strcmp("autostop", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.autostop = (!strcmp("on", tokens[1]) || !strcmp("1", tokens[1])) ? 1 : 0;
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Limiares da parada automática
  if (!strcmp("cstop", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.cStop[0] = atof(tokens[1]);
    glb->stateOut.cStop[1] = atof(tokens[2]);
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Coeficientes de ponderação do PID
  if (!strcmp("cpid", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.cPID[0] = atof(tokens[1]);
    glb->stateOut.cPID[1] = atof(tokens[2]);
    glb->stateOut.cPID[2] = atof(tokens[3]);
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Heater value (0-255)
  if (!strcmp("heat", tokens[0])) {

    // Change state
    pthread_mutex_lock(&(glb->stateOut_mut));
    glb->stateOut.heat = atoi(tokens[1]);
    glb->stateOut.heat = (glb->stateOut.heat < 0) ? 0 : glb->stateOut.heat;
    glb->stateOut.heat = (glb->stateOut.heat > 255) ? 255 : glb->stateOut.heat;
    pthread_mutex_unlock(&(glb->stateOut_mut));
    deviceStateWrite(glb);

    tokens_cleanup(tokens);
    return 0;
  }

  // Fetch state
  if (!strcmp("show", tokens[0])) {

    pthread_mutex_lock(&(glb->state_mut));
    memcpy(&stateLocal, &(glb->state), sizeof(struct State));
    pthread_mutex_unlock(&(glb->state_mut));
    pthread_mutex_lock(&(glb->tempProbe_mut));
    memcpy(tempProbeLocal, glb->tempProbe, 4 * sizeof(float));
    pthread_mutex_unlock(&(glb->tempProbe_mut));

    printf("elapsed           → %d\n", (int)stateLocal.elapsed);
    printf("tempCore          → %.2f\n", stateLocal.tempCore);
    printf("tempEx            → %.2f\n", stateLocal.tempEx);
    printf("tempTarget        → %d\n", (int)stateLocal.tempTarget);
    printf("analogTherm       → %d\n", (int)stateLocal.analogTherm);
    printf("cTemp             → %f %f %f %f\n", stateLocal.cTemp[0], stateLocal.cTemp[1], stateLocal.cTemp[2], stateLocal.cTemp[3]);
    printf("autostop          → %d\n", (int)stateLocal.autostop);
    printf("sStop             → %f %f\n", stateLocal.sStop[0], stateLocal.sStop[1]);
    printf("cStop             → %f %f\n", stateLocal.cStop[0], stateLocal.cStop[1]);
    printf("on                → %d\n", (int)stateLocal.on);
    printf("fan               → %d\n", (int)stateLocal.fan);
    printf("cPID              → %f %f %f\n", stateLocal.cPID[0], stateLocal.cPID[1], stateLocal.cPID[2]);
    printf("PID               → %f %f %f %f %f\n", stateLocal.PID[0], stateLocal.PID[1], stateLocal.PID[2], stateLocal.PID[3], stateLocal.PID[4]);
    printf("PID_enabled       → %d\n", (int)stateLocal.PID_enabled);
    printf("ts                → %ld\n", stateLocal.ts);
    printf("tempStep          → %d\n", (int)stateLocal.tempStep);
    printf("targetLastChange  → %ld\n", stateLocal.targetLastChange);
    printf("splash            → %d\n", (int)stateLocal.splash);
    printf("screensaver       → %d\n", (int)stateLocal.screensaver);
    printf("tempProbe         → %.2f %.2f %.2f %.2f\n", tempProbeLocal[0], tempProbeLocal[1], tempProbeLocal[2], tempProbeLocal[3]);

    tokens_cleanup(tokens);
    return 0;
  }

  // Encerrar
  if (!strcmp("exit", tokens[0]) || !strcmp("end", tokens[0])) {

    tokens_cleanup(tokens);
    return 1;
  }

  /*
   * External commands
   */
  external_cmd(tokens, glb);

  tokens_cleanup(tokens);
  return 0;
}
