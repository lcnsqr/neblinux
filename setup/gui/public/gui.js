/*
 * Preparação do gráficos
 */

// Definições para calibragem
var calibIndex = -1           // Ponto de calibragem ativo
var calibEnabled = 0          // Calibragem em andamento
var calibPointsLabels = []    // Valor do ponto em texto
// Usar os campos de calibragem presentes no documento para preencher as arrays
document.querySelectorAll("#calibPoints input[type='number'].heat").forEach((p) => {
  calibPointsLabels.push(p.value)
})

if (localStorage.getItem("calibHeat") !== null){
  calibPointsLabels = JSON.parse(localStorage.getItem("calibHeat"))
  // Atualizar nos campos
  document.querySelectorAll("#calibPoints input[type='number'].heat").forEach((p) => {
    p.value = calibPointsLabels[p.dataset.index]
  })
}

var calibPointsCore = [ 0, 0, 0, 0, 0, 0, 0, 0 ]
if (localStorage.getItem("calibCore") !== null){
  calibPointsCore = JSON.parse(localStorage.getItem("calibCore"))
}

var calibPointsProbe = [ 0, 0, 0, 0, 0, 0, 0, 0 ]
if (localStorage.getItem("calibProbe") !== null){
  calibPointsProbe = JSON.parse(localStorage.getItem("calibProbe"))
}


// Intervalo de atualização do estado via Websocket
const updateInterval = 100 // milliseconds

// Os gráficos de temperatura e aquecimemto exibem os últimos chartHistorySize pontos.
// O tempo de duração do histórico depende do intervalo de atualização.
// A lista de pontos está no formato [ {x: ..., y: ...}, ... ]
// Ao iniciar, os pontos do histórico estão zerados.

// Valores iniciais para cada gráfico de histórico
const chartHistorySize = 300
var chartData = new Array(5)
for (let j = 0; j < 5; ++j){
  chartData[j] = new Array(chartHistorySize)
  for (let i = 0; i < chartHistorySize; ++i){
    chartData[j][i] = {x: ((chartHistorySize*updateInterval/1000)*(- chartHistorySize + 1 + i)/chartHistorySize).toFixed(2).toString(), y: 0}
  }
}

// Gráficos das temperaturas Alvo e Saída (estimada por um polinômio aproximador de 4ª ordem)
var tempChartA = new Chart(document.getElementById('tempChartA'), {
	type: 'line',
	data: {
		datasets: [
      {
        label: 'Alvo',
        data: chartData[0],
        borderWidth: 1
      },
      {
        label: 'Saída',
        data: chartData[1],
        borderWidth: 1
      }
		]
	},
	options: {
    aspectRatio: 2,
    elements: {
      point: {
        radius: 0
      }
    },
    plugins: {
      title: {
        display: false,
        text: 'Temperaturas Alvo e Saída'
      }
    },
		scales: {
			y: {
				min: 0,
				max: 400
			}
		}
	}
});

// Gráficos das temperaturas Prévia (tomada antes da resistência) e Sonda (na saída)
var tempChartB = new Chart(document.getElementById('tempChartB'), {
	type: 'line',
	data: {
		datasets: [
      {
        label: 'Prévia',
        data: chartData[2],
        borderWidth: 1
      },
      {
        label: 'Sonda',
        data: chartData[3],
        borderWidth: 1
      }
		]
	},
	options: {
    aspectRatio: 2,
    elements: {
      point: {
        radius: 0
      }
    },
    plugins: {
      title: {
        display: false,
        text: 'Temperaturas Prévia e Sonda'
      }
    },
		scales: {
			y: {
				min: 0,
				max: 400
			}
		}
	}
});

// Gráfico da carga na resistência
var heatChart = new Chart(document.getElementById('heatChart'), {
	type: 'line',
	data: {
		datasets: [
      {
        label: 'Carga',
        data: chartData[4],
        borderWidth: 1
      }
		]
	},
	options: {
    elements: {
      point: {
        radius: 0
      }
    },
    plugins: {
      title: {
        display: false,
        text: 'Aquecimento'
      }
    },
		scales: {
			y: {
				min: 0,
				max: 255
			}
		}
	}
});

// Gráfico dos pontos de calibragem e temperaturas aferidas correspondentes
var calibChart = new Chart(document.getElementById('calibChart'), {
	data: {
    labels: calibPointsLabels,
		datasets: [
      {
        label: 'Prévia',
        type: 'line',
        data: calibPointsCore,
        borderWidth: 1
      },
      {
        label: 'Aferida',
        type: 'line',
        data: calibPointsProbe,
        borderWidth: 1
      }
		]
	},
	options: {
    elements: {
      point: {
        radius: 5
      }
    },
    plugins: {
      title: {
        display: false,
        text: 'Calibragem'
      }
    },
		scales: {
      x: {
        min: 0,
        max: 255
      },
			y: {
				min: 0,
				max: 400
			}
		}
	}
});

// Barras de estabilidade recente em core, sonda e carga
var derivChart = new Chart(document.getElementById('derivChart'), {
  type: 'bar',
	data: {
  labels: ['Prévia', 'Sonda', 'Carga'],
		datasets: [
      {
        label: 'Estável se próximo a zero',
        data: [0, 0, 0],
        borderWidth: 1
      }
		]
	},
	options: {
    plugins: {
      title: {
        display: false,
        text: 'Estabilidade recente'
      }
    },
		scales: {
			y: {
				min: -0.1,
				max: 0.1
			}
		}
	}
});

// Barras com coeficientes linear e angular para para automática
var autostopChart = new Chart(document.getElementById('autostopChart'), {
  type: 'bar',
	data: {
  labels: ['Temperatura', 'Aquecimento'],
		datasets: [
      {
        label: 'Coeficientes lineares no tempo',
        data: [0, 0],
        borderWidth: 1
      }
		]
	},
	options: {
    plugins: {
      title: {
        display: false,
        text: 'Regressão linear da temperatura e aquecimento'
      }
    },
		scales: {
			y: {
				min: -0.04,
				max: 0.04
			}
		}
	}
});

/*
 * Preparação dos controles
 */

// Execução de comando via XHR
function exec(command){
  var xhr = new XMLHttpRequest();
  xhr.open('GET', "/command/"+command+"/"+Date.now())
  xhr.onload = function() {
    if (xhr.status === 204) {
      //console.log('Request accepted')
    }
    else {
      console.log('Request failed. Return code: ' + xhr.status)
    }
  }
  xhr.send()
}

// Comportamento comum a todos os campos numéricos de estado
document.querySelectorAll(".stateNumInput").forEach((input) => {
  let actions = ["mousedown", "wheel"]
  actions.forEach((action) => {
    input.addEventListener(action, (e) => {
      e.srcElement.dataset['changed'] = 1
    })
  })
  input.addEventListener("keydown", (e) => {
    // Keycode 13 = Enter
    if (e.keyCode != 13)
      e.srcElement.dataset['changed'] = 1
  })
})

// Ventoinha on/off
document.getElementById("fanControl").addEventListener("change", (e) => {
  e.srcElement.dataset['changed'] = 1
  exec((e.srcElement.checked) ? "on" : "off")
})

// PID on/off
document.getElementById("pidEnabled").addEventListener("change", (e) => {
  e.srcElement.dataset['changed'] = 1
  exec((e.srcElement.checked) ? "pid on" : "pid off")
})

// Modificar temperatura alvo do PID
document.getElementById('target').addEventListener("keydown", (e) => {
  if (e.keyCode == 13)
    exec("target " + e.srcElement.value)
})

// Modificar carga na resistência quando PID desativado
document.getElementById('heatLoad').addEventListener("keydown", (e) => {
  if (e.keyCode == 13 && !document.querySelector("input#pidEnabled").checked)
    exec("heat " + e.srcElement.value)
})

// Parâmetros do PID
document.querySelectorAll(".cPID").forEach((input) => {
  input.addEventListener("keydown", (e) => {
    if (e.keyCode == 13)
      exec("cpid"
        + " " + document.getElementById("cPID0").value
        + " " + document.getElementById("cPID1").value
        + " " + document.getElementById("cPID2").value)
  })
})

// Coeficientes de temperatura
document.querySelectorAll(".cTemp").forEach((input) => {

  input.addEventListener("keydown", (e) => {
    if (e.keyCode == 13){

      exec("ctemp"
        + " " + document.getElementById("cTemp0").value
        + " " + document.getElementById("cTemp1").value
        + " " + document.getElementById("cTemp2").value
        + " " + document.getElementById("cTemp3").value)

      // Atualizar gráfico de calibragem ao mexer nos coeficientes
      for(let i = 0; i < calibPointsCore.length; ++i){
        calibChart.data.datasets[1].data[i] = 
            Number(document.getElementById("cTemp0").value)
          + Number(document.getElementById("cTemp1").value) * calibChart.data.datasets[0].data[i]
          + Number(document.getElementById("cTemp2").value) * Math.pow(calibChart.data.datasets[0].data[i], 2)
          + Number(document.getElementById("cTemp3").value) * Math.pow(calibChart.data.datasets[0].data[i], 3)
      }
      calibChart.update()

    }
  })

})

// Autostop on/off
document.getElementById("autostop").addEventListener("change", (e) => {
  e.srcElement.dataset['changed'] = 1
  exec((e.srcElement.checked) ? "autostop on" : "autostop off")
})

// Passo do giro
document.getElementById('tempstep').addEventListener("keydown", (e) => {
  if (e.keyCode == 13)
    exec("tempstep " + e.srcElement.value)
})

// Screensaver on/off
document.getElementById("screensaver").addEventListener("change", (e) => {
  e.srcElement.dataset['changed'] = 1
  exec((e.srcElement.checked) ? "screensaver on" : "screensaver off")
})

// Limiares da parada automática
document.querySelectorAll(".cStop").forEach((input) => {
  input.addEventListener("keydown", (e) => {
    if (e.keyCode == 13)
      exec("cstop"
        + " " + document.getElementById("cStop0").value
        + " " + document.getElementById("cStop2").value)
  })
})

document.querySelector('button#calibSwitch').addEventListener("click", function(event){
  if ( ! calibEnabled ) return
  if ( this.dataset.state == '1' ){
    exec("heat 0")
    this.dataset.state = 0
    this.innerHTML = "Iniciar"
    calibIndex = -1
  }
  else {
    this.dataset.state = 1
    this.innerHTML = "Parar"
    document.querySelectorAll('#calibPoints input[type="radio"][name="index"]').forEach((p) => {
      if ( p.checked ){
        calibIndex = p.value
        console.log(p.dataset.heat)
        exec("heat "+p.dataset.heat)
      }
    })
  }
})

document.querySelector('button#calibUse').addEventListener("click", function(event){
  // Pontos de calibragem (ponto extra para ancoragem em 25°C)
  const CALIB_POINTS = calibPointsLabels.length + 1
  // Floats de 4 bytes
  let calibCore = _malloc(CALIB_POINTS*4)
  let calibProbe = _malloc(CALIB_POINTS*4)
  // Coeficientes do polinômio interpolador de grau 3
  const CALIB_COEFS = 4
  let cTemp = _malloc(CALIB_COEFS*4)

  // Ponto fixo
  setValue(calibCore, 20.0, 'float')
  setValue(calibProbe, 20.0, 'float')
  for (let i = 1; i < CALIB_POINTS; i++) {
    setValue(calibCore+i*4, calibChart.data.datasets[0].data[i-1], 'float')
    setValue(calibProbe+i*4, calibChart.data.datasets[1].data[i-1], 'float')
  }
  // Computar os coeficientes do polinômio interpolador
  _mat_leastsquares(CALIB_POINTS, CALIB_COEFS - 1, calibCore, calibProbe, cTemp);

  for (let c = 0; c < CALIB_COEFS; c++)
    console.log(getValue(cTemp+c*4, 'float'))

  exec("ctemp "+getValue(cTemp+0*4, 'float')+
    " "+getValue(cTemp+1*4, 'float')+
    " "+getValue(cTemp+2*4, 'float')+
    " "+getValue(cTemp+3*4, 'float'))

  _free(calibCore)
  _free(calibProbe)
  _free(cTemp)

})

document.querySelectorAll('#calibPoints input[type="radio"][name="index"]').forEach((p) => {
  p.addEventListener("change", function(event){
    if ( document.querySelector('button#calibSwitch').dataset.state == "1" && this.checked ){
      calibIndex = this.value
      console.log(this.dataset.heat)
      exec("heat "+this.dataset.heat)

      // Se probe manual, focar no input correspondente
      if ( document.querySelector("input#calibManual").checked ) {
        document.querySelector('#calibPoints input[type="number"][name="cpm'+calibIndex+'"].manual').focus()
      }

    }

  })
})

document.querySelector('#calibPoints div:first-child input[type="radio"]').checked = 'true'

document.querySelectorAll('#calibPoints input[type="number"].heat').forEach((p) => {
  p.addEventListener("change", function(event){

    document.querySelector('#calibPoints input[type="radio"][value="'+this.dataset.index+'"]').dataset.heat = this.value

    if ( document.querySelector('button#calibSwitch').dataset.state == "1" 
      && document.querySelector('#calibPoints input[type="radio"][value="'+this.dataset.index+'"]').checked )
    {
      exec("heat "+this.value)
    }

    calibChart.data.labels[this.dataset.index] = this.value

    calibChart.update('none')

  })
})

// Calibragem manual
document.querySelector("input#calibManual").addEventListener("change", function(event){
  if (this.checked) {
    document.querySelectorAll('#calibPoints input[type="number"].manual').forEach((p) => {
      p.disabled = false
    })
  }
  else {
    document.querySelectorAll('#calibPoints input[type="number"].manual').forEach((p) => {
      p.disabled = true
    })
  }
})

document.querySelectorAll('#calibPoints input[type="number"].manual').forEach((p) => {
  p.value = Math.round(Number(calibChart.data.datasets[1].data[p.dataset.index]))

  // Saltar para o ponto correspondente ao ganhar foco
  p.addEventListener("focus", function(event){

    let rp = document.querySelector('#calibPoints input[type="radio"][value="'+this.dataset.index+'"]')
    if ( ! rp.checked ){
      rp.checked = true
    }

  })

  // Atualizar gráfico ao mudar o valor manualmente
  p.addEventListener("change", function(event){
    // Se probe manual, usar valor no input correspondente
    if ( document.querySelector("input#calibManual").checked ) {
      calibChart.data.datasets[1].data[this.dataset.index] = this.value
      calibChart.update()
    }
  })
})

document.querySelector('button#eepromReset').addEventListener("click", function(event){
  if (window.confirm("Apagar as definições atuais da EEPROM?")) {
    exec("reset")
  }
})

document.querySelector('button#eepromStore').addEventListener("click", function(event){

  exec("store")

  // Preservar valores em tela para futuras sessões
  localStorage.setItem("calibCore", JSON.stringify(calibChart.data.datasets[0].data))
  localStorage.setItem("calibProbe", JSON.stringify(calibChart.data.datasets[1].data))
  localStorage.setItem("calibHeat", JSON.stringify(calibChart.data.labels))
})

/*
 * Comunicação com o backend
 */

// Espera carregamento WebAssembly
var Module = {
	onRuntimeInitialized: function() {

    // Conexão WebSocket com backend para obter estado
    guisock = new WebSocket('ws://127.0.0.1:8888')

    // Iniciar conversa ao abrir conexão
    guisock.onopen = function(event){
      // Solicitação recorrente de estado
      setInterval(() => {
        guisock.send("state")
      }, updateInterval)
    }

    // Tratamento à estrutura de dados recebida via Websocket
    guisock.onmessage = function(event){
      var data = JSON.parse(event.data)

      /*
       * Gráficos
       */

      // Colocar temperaturas da sonda quádrupla em ordem decrescente
      const tempProbe = data.tempProbe.toSorted((a, b) => b - a)
      // Usar média da quatro leituras da sonda
      const tempProbeValue = (0.25*(tempProbe[0]+tempProbe[1]+tempProbe[2]+tempProbe[3])).toFixed(2)

      // Deslocar pontos dos gráficos de histórico para trás
      for (let i = 0; i < chartHistorySize - 1; ++i){
        // Gráfico de temperaturas Alvo e Saída
        tempChartA.data.datasets[0].data[i].y = tempChartA.data.datasets[0].data[i+1].y
        tempChartA.data.datasets[1].data[i].y = tempChartA.data.datasets[1].data[i+1].y
        // Gráfico de temperatura Prévia e Sonda
        tempChartB.data.datasets[0].data[i].y = tempChartB.data.datasets[0].data[i+1].y
        tempChartB.data.datasets[1].data[i].y = tempChartB.data.datasets[1].data[i+1].y
        // Gráfico de carga na resistência
        heatChart.data.datasets[0].data[i].y = heatChart.data.datasets[0].data[i+1].y
      }

      // Gráfico de temperaturas alvo e saída
      tempChartA.data.datasets[0].label = "Alvo: "+data.tempTarget+" °C"
      tempChartA.data.datasets[1].label = "Saída: "+Math.round(Number(data.tempEx))+" °C"
      tempChartA.data.datasets[0].data[chartHistorySize-1].y = data.tempTarget
      tempChartA.data.datasets[1].data[chartHistorySize-1].y = data.tempEx
      tempChartA.update('none')

      // Gráfico de temperatura Prévia e Sonda
      tempChartB.data.datasets[0].label = "Prévia: "+Math.round(Number(data.tempCore))+" °C"
      tempChartB.data.datasets[1].label = "Sonda: "+Math.round(tempProbeValue)+" °C"
      tempChartB.data.datasets[0].data[chartHistorySize-1].y = data.tempCore
      tempChartB.data.datasets[1].data[chartHistorySize-1].y = tempProbeValue
      tempChartB.update('none')

      // Gráfico de carga na resistência
      heatChart.data.datasets[0].label = "Carga: " + Math.round(Number(data.PID[4]));
      heatChart.data.datasets[0].data[chartHistorySize-1].y = data.PID[4]
      heatChart.update('none')

      // Pontos de calibragem
      if ( calibEnabled && document.querySelector('button#calibSwitch').dataset.state == "1" ){
        calibChart.data.datasets[0].data[calibIndex] = data.tempCore

        // Utilizar temperatura da sonda se não for manual
        if ( ! document.querySelector("input#calibManual").checked ) {
          calibChart.data.datasets[1].data[calibIndex] = (0.25*(tempProbe[0]+tempProbe[1]+tempProbe[2]+tempProbe[3])).toFixed(2)
        }

        calibChart.update()
      }

      // Estabilidade recente
      // Regressão linear nos 20 últimos pontos
      const TAIL_POINTS = 30

      // Máximos para normalização dos domínios de temperatura e carga
      const TEMP_MAX = 400.0
      const HEAT_MAX = 255.0

      // Floats de 4 bytes
      let tailX = _malloc(TAIL_POINTS*4)

      let tailCore = _malloc(TAIL_POINTS*4)
      let lineCore = _malloc(2*4)
      let tailProbe = _malloc(TAIL_POINTS*4)
      let lineProbe = _malloc(2*4)
      let tailHeat = _malloc(TAIL_POINTS*4)
      let lineHeat = _malloc(2*4)

      for (let i = 0; i < TAIL_POINTS; i++){
        // Pontos no domínio entre 0 e 1
        setValue(tailX+i*4, i/TAIL_POINTS, 'float')
        // Prévia
        setValue(tailCore+i*4, tempChartB.data.datasets[0].data[chartHistorySize-TAIL_POINTS+i].y / TEMP_MAX, 'float')
        // Sonda
        setValue(tailProbe+i*4, tempChartB.data.datasets[1].data[chartHistorySize-TAIL_POINTS+i].y / TEMP_MAX, 'float')
        // Aquecedor
        setValue(tailHeat+i*4, heatChart.data.datasets[0].data[chartHistorySize-TAIL_POINTS+i].y / HEAT_MAX, 'float')
      }

      _mat_leastsquares(TAIL_POINTS, 1, tailX, tailCore, lineCore);
      _mat_leastsquares(TAIL_POINTS, 1, tailX, tailProbe, lineProbe);
      _mat_leastsquares(TAIL_POINTS, 1, tailX, tailHeat, lineHeat);

      derivChart.data.datasets[0].data = [
        getValue(lineCore+4, 'float'),
        getValue(lineProbe+4, 'float'),
        getValue(lineHeat+4, 'float')
      ]
      derivChart.update('none')

      _free(tailX)
      _free(tailCore)
      _free(lineCore)
      _free(tailProbe)
      _free(lineProbe)
      _free(tailHeat)
      _free(lineHeat)

      // Componentes linear a angular da reta que aproxima
      // os pontos recentes da carga na resistência
      if ( Boolean(Number(data.on)) && Boolean(Number(data.PID_enabled)) ){
        // Atualizar apenas se PID ativo e soprando
        autostopChart.data.labels[0] = "Temperatura: "+data.sStop[0]
        autostopChart.data.labels[1] = "Aquecimento: "+data.sStop[1]
        autostopChart.data.datasets[0].data = [
          data.sStop[0],
          data.sStop[1]
        ]
        autostopChart.update('none')
      }

      /*
       * Controles
       */

      // Assoprar e esquentar se PID ativo
      let input = document.getElementById('fanControl')
      let value = Boolean(Number(data.on))
      if ( input.dataset['changed'] != 1 ) // Se não foi modificado pelo usuário
        input.checked = value // usar valor recebido do aparelho
      else if ( input.checked == value ) // Se modificado pelo usuário e valor igual ao recebido do aparelho
        input.dataset['changed'] = 0 // marcar como não modificado pelo usuário

      // Campo do valor da carga na ventoinha
      input = document.getElementById('fanLoad')
      value = Math.round(Number(data.fan))
      if ( input.dataset['changed'] != 1 )
        input.value = value
      else if ( input.value == value )
        input.dataset['changed'] = 0

      // Duração do último acionamento
      document.getElementById('elapsed').innerHTML = Math.floor(Number(data.elapsed)/60) + "m" + Math.floor(Number(data.elapsed)%60) + "s"

      // Ativação PID
      input = document.getElementById('pidEnabled')
      value = Boolean(Number(data.PID_enabled))
      if ( input.dataset['changed'] != 1 )
        input.checked = value
      else if ( input.checked == value )
        input.dataset['changed'] = 0

      // Campo do valor da carga na resistência
      input = document.getElementById('heatLoad')
      value = Math.round(Number(data.PID[4]))
      if ( input.dataset['changed'] != 1 )
        input.value = value
      else if ( input.value == value )
        input.dataset['changed'] = 0

      // Bloquear carga na resistência se PID ativo
      if ( Boolean(Number(data.PID_enabled)) )
        input.disabled = true
      else
        input.disabled = false

      // Alvo do PID
      input = document.getElementById('target')
      if ( input.dataset['changed'] != 1 )
        input.value = data.tempTarget
      else if ( input.value == data.tempTarget )
        input.dataset['changed'] = 0

      // Coeficientes PID
      for (let i = 0; i < 3; i++){
        input = document.getElementById('cPID'+String(i))
        if ( input.dataset['changed'] != 1 )
          input.value = Number(data.cPID[i])
        else if ( input.value == Number(data.cPID[i]) )
          input.dataset['changed'] = 0
      }

      // Coeficientes do polinômio estimador de temperatura
      for (let i = 0; i < 4; i++){
        input = document.getElementById('cTemp'+String(i))
        if ( input.dataset['changed'] != 1 )
          input.value = Number(data.cTemp[i])
        else if ( input.value == Number(data.cTemp[i]) )
          input.dataset['changed'] = 0
      }

      // Ativação Autostop
      input = document.getElementById('autostop')
      value = Boolean(Number(data.autostop))
      if ( input.dataset['changed'] != 1 )
        input.checked = value
      else if ( input.checked == value )
        input.dataset['changed'] = 0

      // Passo do giro
      input = document.getElementById('tempstep')
      if ( input.dataset['changed'] != 1 )
        input.value = data.tempStep
      else if ( input.value == data.tempStep )
        input.dataset['changed'] = 0

      // Descanso de tela
      input = document.getElementById('screensaver')
      value = Boolean(Number(data.screensaver))
      if ( input.dataset['changed'] != 1 )
        input.checked = value
      else if ( input.checked == value )
        input.dataset['changed'] = 0

      // Limiares da parada automática
      for (let i = 0; i < 2; i++){
        input = document.getElementById('cStop'+String(i))
        if ( input.dataset['changed'] != 1 )
          input.value = Number(data.cStop[i])
        else if ( input.value == Number(data.cStop[i]) )
          input.dataset['changed'] = 0
      }


      //document.querySelector('#state td[data-id="pid0"]').innerHTML = data.PID[0];
      //document.querySelector('#state td[data-id="pid1"]').innerHTML = data.PID[1];
      //document.querySelector('#state td[data-id="pid2"]').innerHTML = data.PID[2];

      if ( Boolean(Number(data.on)) && !Boolean(Number(data.PID_enabled)) ){
        // Liberar calibragem
        calibEnabled  = 1
        document.querySelector('button#calibSwitch').disabled = false
        document.querySelector('button#calibUse').disabled = false
      }
      else {
        calibEnabled  = 0
        document.querySelector('button#calibSwitch').disabled = true
        document.querySelector('button#calibUse').disabled = true
      }

    }

  }
}
