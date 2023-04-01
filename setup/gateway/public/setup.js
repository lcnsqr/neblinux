var temp = new Chart(document.getElementById('temp'), {
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


var heat = new Chart(document.getElementById('heat'), {
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

var calib = new Chart(document.getElementById('calib'), {
	data: {
		datasets: [
      {
        label: 'Interna',
        type: 'scatter',
        data: [{x: '0', y: 25},{x: '50', y: 30},{x: '150', y: 80},{x: '200', y: 100},{x: '255', y: 120}],
        borderWidth: 1
      },
      {
        label: 'Sonda',
        type: 'scatter',
        data: [{x: '0', y: 33},{x: '50', y: 35},{x: '150', y: 110},{x: '200', y: 221},{x: '255', y: 240}],
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
			y: {
				min: 0,
				max: 400
			}
		}
	}
});


var deriv = new Chart(document.getElementById('deriv'), {
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
  temp.data.datasets[0].data = data.graph.target
  temp.data.datasets[1].data = data.graph.core
  temp.data.datasets[2].data = data.graph.probe
  temp.update('none')
  heat.data.datasets[0].data = data.graph.heat
  heat.update('none')

  // Estado
  document.querySelector('#state td[data-id="on"]').innerHTML = (data.on != 0) ? "Sim" : "Não";
  document.querySelector('#state td[data-id="elapsed"]').innerHTML = data.elapsed;
  document.querySelector('#state td[data-id="fan"]').innerHTML = (data.fan != 0) ? "Sim" : "Não";
  document.querySelector('#state td[data-id="target"]').innerHTML = data.graph.target[data.graph.target.length-1].y;
  document.querySelector('#state td[data-id="core"]').innerHTML = data.graph.core[data.graph.core.length-1].y;
  document.querySelector('#state td[data-id="ex"]').innerHTML = data.graph.ex[data.graph.ex.length-1].y;
  document.querySelector('#state td[data-id="probe"]').innerHTML = data.graph.probe[data.graph.probe.length-1].y;
  document.querySelector('#state td[data-id="heat"]').innerHTML = data.graph.heat[data.graph.heat.length-1].y;

  document.querySelector('#state td[data-id="pid0"]').innerHTML = data.PID[0];
  document.querySelector('#state td[data-id="pid1"]').innerHTML = data.PID[1];
  document.querySelector('#state td[data-id="pid2"]').innerHTML = data.PID[2];
}
