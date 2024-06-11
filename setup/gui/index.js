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

// EJS
app.set('view engine', 'ejs')

// Pontos de calibragem
const calibPoints = 8
var calibPointsValues = []
for (var i = 0; i < calibPoints; i++)
  calibPointsValues.push(10 + Math.floor(170 * Math.sin(Math.PI * i/(calibPoints*2))))

app.get('/', (req, res) => {
  res.render('main', {title: "Configuração remota", calibPoints: calibPoints, calibPointsValues: calibPointsValues})
})

app.get('/command/:command/:timestamp', (req, res) => {
  // Enviar comando via unix socket
  const client = net.createConnection({path: 'socket'}, () => {
    client.write(req.params.command)
    client.on('data', (data) => {
    // Apenas fechar conexão por socket e ignorar resposta
      client.end()
    })
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

// Processo principal envia um sinal SIGTERM para encerrar
process.on('SIGTERM', () => {
  server.close()
  wss.close()
  process.exit()
})

// Websocket
wss.on('connection', (ws) => {
  ws.on('error', () => {
    console.log("Erro na conexão Web socket")
  })

  ws.on('message', (data) => {
    if ( data == "state" ){
      // Solicitar estado ao processo principal via
      // Unix socket e responder à interface via websocket
      const client = net.createConnection({path: 'socket'}, () => {
        client.on('data', (data) => {
          // Enviar resposta por websocket
          ws.send(data.toString())
          client.end()
        })
        client.on('error', () => {
          client.end()
        })
        client.write('STATE')
      })
    }
  })

})
