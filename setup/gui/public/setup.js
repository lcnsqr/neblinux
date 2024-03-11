// Ponto de calibragem ativo
var calibIndex = -1
var calibEnabled = 0
var calibPointsLabels = []
var calibPointsValues = []
document.querySelectorAll("#calibPoints input[type='number'].heat").forEach((p) => {
  calibPointsLabels.push(p.value)
  calibPointsValues.push(Number(p.value))
})

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

var tempChart = new Chart(document.getElementById('tempChart'), {
	type: 'line',
	data: {
		datasets: [
      {
        label: 'Alvo',
        data: [],
        borderWidth: 1
      },
      {
        label: 'Interna',
        data: [],
        borderWidth: 1
      },
      {
        label: 'Sonda',
        data: [],
        borderWidth: 1
      }
		]
	},
	options: {
    aspectRatio: 1,
    elements: {
      point: {
        radius: 0
      }
    },
    plugins: {
      title: {
        display: true,
        text: 'Temperaturas'
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


var heatChart = new Chart(document.getElementById('heatChart'), {
	type: 'line',
	data: {
		datasets: [
      {
        label: 'Carga',
        data: [],
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
        display: true,
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

var calibChart = new Chart(document.getElementById('calibChart'), {
	data: {
    labels: calibPointsLabels,
		datasets: [
      {
        label: 'Interna',
        type: 'line',
        data: [ 31.0717, 40.1702, 53.7266, 70.8701, 90.0546, 111.3144, 126.8766, 142.917 ],
        borderWidth: 1
      },
      {
        label: 'Aferida',
        type: 'line',
        data: [ 54, 103, 153, 196, 241, 279, 302, 325 ],
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
        display: true,
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


var derivChart = new Chart(document.getElementById('derivChart'), {
  type: 'bar',
	data: {
  labels: ['Interna', 'Sonda', 'Carga'],
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
        display: true,
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

// Conexão WebSocket
ws = new WebSocket('ws://127.0.0.1:8888')

ws.onopen = function(event){
  //console.log("Conexão websocket aberta")
}

ws.onmessage = function(event){
  var data = JSON.parse(event.data)

  // Gráficos
  tempChart.data.datasets[0].data = data.graph.target
  tempChart.data.datasets[1].data = data.graph.core
  tempChart.data.datasets[2].data = data.graph.probe
  tempChart.update('none')
  heatChart.data.datasets[0].data = data.graph.heat
  heatChart.update('none')

  // Calibragem
  if ( calibEnabled && document.querySelector('button#calibSwitch').dataset.state == "1" ){
    calibChart.data.datasets[0].data[calibIndex] = data.graph.core[data.graph.core.length-1].y

    // Utilizar temperatura da sonda se não for manual
    if ( ! document.querySelector("input#calibManual").checked ) {
      calibChart.data.datasets[1].data[calibIndex] = data.graph.probe[data.graph.probe.length-1].y
    }

    calibChart.update()
  }

  // Estabilidade recente
  derivChart.data.datasets[0].data = [data.deriv.core, data.deriv.probe, data.deriv.heat]
  derivChart.update('none')

  // Estado
  document.querySelector('#state td[data-id="on"]').innerHTML = (data.on != 0) ? "Sim" : "Não";

  // Liberar calibragem
  if ( data.on != 0 && data.PID_enabled == 0 ){
    document.querySelectorAll('form#calibPoints input[type="radio"][name="index"]').forEach((p) => {
      p.disabled = false
    })
    document.querySelectorAll('.calibButton').forEach((b) => {
      b.disabled = false
    })
    calibEnabled = 1
  }
  else {
    if ( calibEnabled == 1 ) exec("heat 0")
    document.querySelectorAll('form#calibPoints input[type="radio"][name="index"]').forEach((p) => {
      p.disabled = true
    })
    document.querySelectorAll('.calibButton#calibSwitch').forEach((b) => {
      b.disabled = true
    })
    calibEnabled = 0
  }


  document.querySelector('#state td[data-id="fan"]').innerHTML = data.fan;

  document.querySelector('#state td[data-id="elapsed"]').innerHTML = data.elapsed;

  document.querySelector('#state td[data-id="tempstep"]').innerHTML = data.tempStep;

  document.querySelector('#state td[data-id="target"]').innerHTML = data.graph.target[data.graph.target.length-1].y;
  document.querySelector('#state td[data-id="core"]').innerHTML = data.graph.core[data.graph.core.length-1].y;
  document.querySelector('#state td[data-id="ex"]').innerHTML = data.graph.ex[data.graph.ex.length-1].y;
  document.querySelector('#state td[data-id="probe"]').innerHTML = data.graph.probe[data.graph.probe.length-1].y;
  document.querySelector('#state td[data-id="heat"]').innerHTML = data.graph.heat[data.graph.heat.length-1].y;

  document.querySelector('#state td[data-id="pid0"]').innerHTML = data.PID[0];
  document.querySelector('#state td[data-id="pid1"]').innerHTML = data.PID[1];
  document.querySelector('#state td[data-id="pid2"]').innerHTML = data.PID[2];

  if ( data.PID_enabled != 0 ){
    document.querySelector('#state td[data-id="pid_enabled"]').innerHTML = "Sim";
  }
  else {
    document.querySelector('#state td[data-id="pid_enabled"]').innerHTML = "Não";
  }

  if ( data.autostop != 0 ){
    document.querySelector('#state td[data-id="autostop"]').innerHTML = "Sim";
  }
  else {
    document.querySelector('#state td[data-id="autostop"]').innerHTML = "Não";
  }
  document.querySelector('#state td[data-id="sstop0"]').innerHTML = data.sStop[0];
  document.querySelector('#state td[data-id="sstop1"]').innerHTML = data.sStop[1];

  document.querySelector('#state td[data-id="screensaver"]').innerHTML = (data.screensaver != 0) ? "Sim" : "Não";


  document.querySelector('#settings td[data-id="cTemp0"]').innerHTML = data.cTemp[0];
  document.querySelector('#settings td[data-id="cTemp1"]').innerHTML = data.cTemp[1];
  document.querySelector('#settings td[data-id="cTemp2"]').innerHTML = data.cTemp[2];
  document.querySelector('#settings td[data-id="cTemp3"]').innerHTML = data.cTemp[3];

  document.querySelector('#settings td[data-id="cPID0"]').innerHTML = data.cPID[0];
  document.querySelector('#settings td[data-id="cPID1"]').innerHTML = data.cPID[1];
  document.querySelector('#settings td[data-id="cPID2"]').innerHTML = data.cPID[2];

  document.querySelector('#settings td[data-id="cstop0"]').innerHTML = data.cStop[0];
  document.querySelector('#settings td[data-id="cstop1"]').innerHTML = data.cStop[1];
}

document.querySelector('form#prompt').addEventListener("submit", function(event){
  event.preventDefault()
	var command = this.querySelector('input[name="command"]').value
	if ( command.trim().length == 0 ) return

  exec(command)
  document.querySelector('form#prompt').reset()

})

document.querySelector('form#calibPoints').addEventListener("submit", function(event){
  event.preventDefault()
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
    document.querySelectorAll('form#calibPoints input[type="radio"][name="index"]').forEach((p) => {
      if ( p.checked ){
        calibIndex = p.value
        console.log(p.dataset.heat)
        exec("heat "+p.dataset.heat)
      }
    })
  }
})

document.querySelector('button#calibSave').addEventListener("click", function(event){
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/calib?core="+calibChart.data.datasets[0].data.join(' ')+"&probe="+calibChart.data.datasets[1].data.join(' '))
	xhr.onload = function() {
		if (xhr.status === 204) {
      //console.log('Request accepted')
		}
		else {
			console.log('Request failed. Return code: ' + xhr.status)
		}
	}
	xhr.send()
})

document.querySelectorAll('form#calibPoints input[type="radio"][name="index"]').forEach((p) => {
  p.addEventListener("change", function(event){
    if ( document.querySelector('button#calibSwitch').dataset.state == "1" && this.checked ){
      calibIndex = this.value
      console.log(this.dataset.heat)
      exec("heat "+this.dataset.heat)

      // Se probe manual, focar no input correspondente
      if ( document.querySelector("input#calibManual").checked ) {
        document.querySelector('form#calibPoints input[type="number"][name="cpm'+calibIndex+'"].manual').focus()
      }

    }

  })
})

document.querySelector('form#calibPoints div:first-child input[type="radio"]').checked = 'true'

document.querySelectorAll('form#calibPoints input[type="number"].heat').forEach((p) => {
  p.addEventListener("change", function(event){

    document.querySelector('form#calibPoints input[type="radio"][value="'+this.dataset.index+'"]').dataset.heat = this.value

    if ( document.querySelector('button#calibSwitch').dataset.state == "1" 
      && document.querySelector('form#calibPoints input[type="radio"][value="'+this.dataset.index+'"]').checked )
    {
      console.log(this.value)
      exec("heat "+this.value)
    }

    calibChart.data.labels[this.dataset.index] = this.value

    calibChart.update('none')

  })
})

// Calibragem manual
document.querySelector("input#calibManual").addEventListener("change", function(event){
  if (this.checked) {
    document.querySelectorAll('form#calibPoints input[type="number"].manual').forEach((p) => {
      p.disabled = false
    })
  }
  else {
    document.querySelectorAll('form#calibPoints input[type="number"].manual').forEach((p) => {
      p.disabled = true
    })
  }
})

document.querySelectorAll('form#calibPoints input[type="number"].manual').forEach((p) => {
  p.value = calibChart.data.datasets[1].data[p.dataset.index]
})

document.querySelectorAll('form#calibPoints input[type="number"].manual').forEach((p) => {
  p.addEventListener("change", function(event){
    // Se probe manual, usar valor no input correspondente
    if ( document.querySelector("input#calibManual").checked ) {
      calibChart.data.datasets[1].data[this.dataset.index] = this.value
      calibChart.update()
    }
  })
})
