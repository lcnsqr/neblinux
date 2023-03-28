// HTTP
const express = require('express')
const app = express()
const host = "127.0.0.1"
const port = 8080

// Ler arquivos SVG
const fs = require("fs")
// Cópias dos últimos gráficos lidos corretamente
var graph = {
  temp: '',
  gist: ''
}

// Websockets
const WebSocketServer = require('ws').Server;
const wss = new WebSocketServer({host: '127.0.0.1', port: 8888});

// Unix sockets
const net = require('net');
const socketfile = '/tmp/vapomatic.sock';

app.get('/', (req, res) => {
  fs.readFile('../setup.html', 'utf8', (err, data) => {
    if (err) {
      console.error(err)
      return
    }
    res.send(data)
    return
  })
})

app.get('/graph/:graph(temp|gist).svg', (req, res) => {
  fs.readFile('../'+req.params.graph+'.svg', 'utf8', (err, data) => {
    if (err || data.substr(data.length - 7, 6) != "</svg>" ) {
      res.setHeader("Content-Type", "image/svg+xml")
      res.send(graph[req.params.graph])
      return
    }
    graph[req.params.graph] = data
    res.setHeader("Content-Type", "image/svg+xml")
    res.send(data)
    return
  })
})

const server = app.listen(port, host, () => {
  console.log(`Server ready at http://${host}:${port}`)
})

// SIGINT = CTRL+C
process.on('SIGINT', () => {
  server.close()
  wss.close()
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
    client.write('STATE')
    client.on('data', (data) => {
      // Enviar resposta por websocket
      ws.send(data.toString())
      client.end()
    })
  }, 250)


})
