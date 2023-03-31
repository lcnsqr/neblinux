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

// Conexão WebSocket
ws = new WebSocket('ws://127.0.0.1:8888')

ws.onopen = function(event){
  //console.log("Conexão websocket aberta")
}

ws.onmessage = function(event){
  var data = JSON.parse(event.data)
  temp.data.datasets[0].data = data.graph.target
  temp.data.datasets[1].data = data.graph.core
  temp.data.datasets[2].data = data.graph.probe
  temp.update('none')
  heat.data.datasets[0].data = data.graph.heat
  heat.update('none')
}
