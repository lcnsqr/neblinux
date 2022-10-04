#include "session.h"
#include "mat.h"
#include <EEPROM.h>

Session::Session() {
  dryrun = 1;

  tempeMin = 20.0;
  tempeMax = 240.0;

  changed = false;
  tempeCore = 0;
  tempeEx = 0;
  tempeTarget = 180;

  on = false;

  elapsed = 0;

  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 0;

  shut[0] = 0;
  shut[1] = 0;
}

void Session::load(){

  // Carregar configurações salvas
  EEPROM.get(0, settings);

  // thCfs[0] : Coeficientes usados quando desativado
  thCfs[0][0] = 0;
  thCfs[0][1] = 1.0;
  thCfs[0][2] = 0;

  // Quantidade de pontos de calibragem
  const int m = 3;
  // Temperaturas da calibragem
  /*
	settings.tempCore[0] = 20;
	settings.tempCore[1] = 130;
	settings.tempCore[2] = 180;
	settings.tempEx[0] = 20;
	settings.tempEx[1] = 180;
	settings.tempEx[2] = 240;
  */
  // Grau do polinômio interpolador
  const int n = 2;
  // thCfs[1] : Coeficientes usados quando ativado
  leastsquares(m, n, settings.tempCore, settings.tempEx, thCfs[1]);

  // Coeficientes PID
  /*
  settings.PID[0] = 1e-2;
  settings.PID[1] = 1.5e-4;
  settings.PID[2] = 7e-2;
  */

  // Limiares de desligamento
  /*
  settings.shutLim[0] = 4.0;
  settings.shutLim[1] = 1.0;
  */
}

void Session::save(){
  EEPROM.put(0, settings);
}

int Session::leastsquares(int m, int n, double x[], double y[], double c[]){
  // Número de coeficientes é o grau + 1
  n = n + 1;

  // Matriz para mínimos quadrados
	double A[n*n];
  for (int j = 0; j < n; ++j){
    for (int k = 0; k < n; ++k){
      A[mat::elem(n,n,j,k)] = 0;
      for (int i = 0; i < m; ++i){
        A[mat::elem(n,n,j,k)] += pow(x[i], j+k);
      }
    }
  }

  // Inversa
  double Ainv[n*n];
  // Inverter matriz (perde A)
  if ( mat::inv(n, A, Ainv) ) return -1;
	
  // RHS
  double b[n];
  for (int j = 0; j < n; ++j){
    b[j] = 0;
    for (int i = 0; i < m; ++i){
      b[j] += pow(x[i], j) * y[i];
    }
  }

  // Coeficientes
	mat::mult(n,n,1,Ainv,b,c);
}

void Session::start(){
  on = true;
  changed = true;
}

void Session::stop(){
  on = false;
  changed = true;

  // Resetar PID
  PID[0] = 0;
  PID[1] = 0;
  PID[2] = 0;
  PID[3] = 0;
  PID[4] = 0;
}

bool Session::running(){
  return on;
}
