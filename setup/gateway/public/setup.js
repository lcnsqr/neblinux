// Ponto de calibragem ativo
var calibPoint = -1

var calibPointsLabels = []
var calibPointsValues = []
document.querySelectorAll("#calibPoints input[type='number']").forEach((p) => {
  calibPointsLabels.push(p.value)
  calibPointsValues.push(Number(p.value))
})

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
    aspectRatio: 1,
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
        type: 'scatter',
        data: calibPointsValues,
        borderWidth: 1
      },
      {
        label: 'Sonda',
        type: 'scatter',
        data: calibPointsValues,
        borderWidth: 1
      }
		]
	},
	options: {
    aspectRatio: 1,
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
        label: 'Estabilidade recente',
        data: [0.1, 0.3, -0.5],
        borderWidth: 1
      }
		]
	},
	options: {
    aspectRatio: 1,
    plugins: {
      title: {
        display: true,
        text: 'Estabilidade'
      }
    },
		scales: {
			y: {
				min: -1,
				max: 1
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
  if ( calibPoint > -1 ){
    calibChart.data.datasets[0].data[calibPoint] = data.graph.core[data.graph.core.length-1].y
    calibChart.data.datasets[1].data[calibPoint] = data.graph.probe[data.graph.probe.length-1].y
    calibChart.update()
  }

  // Estado
  document.querySelector('#state td[data-id="on"]').innerHTML = (data.on != 0) ? "Sim" : "Não";
  document.querySelector('#state td[data-id="elapsed"]').innerHTML = data.elapsed;

  if ( data.fan != 0 ){
    document.querySelector('#state td[data-id="fan"]').innerHTML = "Sim"
    document.querySelector('button#calibSwitch').disabled = false
  }
  else {
    document.querySelector('#state td[data-id="fan"]').innerHTML = "Não"
    document.querySelector('button#calibSwitch').disabled = true
  }

  document.querySelector('#state td[data-id="target"]').innerHTML = data.graph.target[data.graph.target.length-1].y;
  document.querySelector('#state td[data-id="core"]').innerHTML = data.graph.core[data.graph.core.length-1].y;
  document.querySelector('#state td[data-id="ex"]').innerHTML = data.graph.ex[data.graph.ex.length-1].y;
  document.querySelector('#state td[data-id="probe"]').innerHTML = data.graph.probe[data.graph.probe.length-1].y;
  document.querySelector('#state td[data-id="heat"]').innerHTML = data.graph.heat[data.graph.heat.length-1].y;

  document.querySelector('#state td[data-id="pid0"]').innerHTML = data.PID[0];
  document.querySelector('#state td[data-id="pid1"]').innerHTML = data.PID[1];
  document.querySelector('#state td[data-id="pid2"]').innerHTML = data.PID[2];
}

document.querySelector('form#prompt').addEventListener("submit", function(event){
  event.preventDefault()
	var command = this.querySelector('input[name="command"]').value
	if ( command.trim().length == 0 ) return

	var xhr = new XMLHttpRequest();
	xhr.open('GET', this.getAttribute("action")+"/"+command+"/"+Date.now())
	xhr.onload = function() {
		if (xhr.status === 204) {
      document.querySelector('form#prompt').reset()
		}
		else {
			console.log('Request failed. Return code: ' + xhr.status)
		}
	}
	xhr.send()
})

document.querySelector('form#calibPoints').addEventListener("submit", function(event){
  event.preventDefault()
})

document.querySelector('button#calibSwitch').addEventListener("click", function(event){
  if ( this.dataset.state == '1' ){
    this.dataset.state = 0
    this.innerHTML = "Iniciar"
  }
  else {
    this.dataset.state = 1
    this.innerHTML = "Finalizar"
  }
})

document.querySelectorAll('form#calibPoints input[type="radio"][name="index"]').forEach((p) => {
  p.addEventListener("change", function(event){
    if ( this.checked )
      calibPoint = this.value
  })
})

document.querySelector('form#calibPoints div:first-child input[type="radio"]').checked = 'true'

document.querySelectorAll('form#calibPoints input[type="number"]').forEach((p) => {
  p.addEventListener("change", function(event){

    calibChart.data.labels[this.dataset.index] = this.value
    calibChart.update('none')

  })
})
