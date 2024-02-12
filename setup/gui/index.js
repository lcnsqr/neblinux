// HTTP
const express = require('express')
const app = express()
const host = "127.0.0.1"
const port = 8080

// Static files
app.use(express.static('public'))

// Websockets
const WebSocketServer = require('ws').Server
const wss = new WebSocketServer({host: '127.0.0.1', port: 8888})

// Unix sockets
const net = require('net')
const socketfile = '/tmp/vapomatic.sock'

// EJS
app.set('view engine', 'ejs')

// Pontos de calibragem
const calibPoints = 8
var calibPointsValues = []
for (var i = 0; i < calibPoints; i++)
  calibPointsValues.push(10 + Math.floor(120 * Math.sin(Math.PI * i/(calibPoints*2))))

app.get('/', (req, res) => {
  res.render('main', {title: "Vapomatic", calibPoints: calibPoints, calibPointsValues: calibPointsValues})
})

app.get('/calib', (req, res) => {
  // Mesclar leituras e enviar comando via unix socket
  const client = net.createConnection({path: socketfile})
  client.write('calib '+req.query.core+' '+req.query.probe)
  client.on('data', (data) => {
  // Apenas fechar conexão por socket e ignorar resposta
    client.end()
  })
  res.sendStatus(204)
})

app.get('/command/:command/:timestamp', (req, res) => {
  // Enviar comando via unix socket
  const client = net.createConnection({path: socketfile})
  client.write(req.params.command)
  client.on('data', (data) => {
  // Apenas fechar conexão por socket e ignorar resposta
    client.end()
  })
  res.sendStatus(204)
})

const server = app.listen(port, host, () => {
  console.log(`GUI disponível em http://${host}:${port}`)
})

// SIGINT = CTRL+C
process.on('SIGINT', () => {
  server.close()
  wss.close()
  process.exit()
})

// Websocket
wss.on('connection', function connection(ws) {
  ws.on('error', console.error)

  ws.on('message', function message(data) {
    console.log('received: %s', data)
  })

  setInterval(() => {
    // Obter sessão via unix socket
    const client = net.createConnection({path: socketfile})
    client.on('error', () => {
      server.close()
      wss.close()
      process.exit()
    })
    client.write('STATE')
    client.on('data', (data) => {
      // Enviar resposta por websocket
      ws.send(data.toString())
      client.end()
    })
  }, 250)


})
