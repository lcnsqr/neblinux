// HTTP
const express = require('express')
const app = express()
const host = "127.0.0.1"
const port = 8080

// Static files
app.use(express.static('public'))

// Websockets
const WebSocketServer = require('ws').Server;
const wss = new WebSocketServer({host: '127.0.0.1', port: 8888});

// Unix sockets
const net = require('net');
const socketfile = '/tmp/vapomatic.sock';

// EJS
app.set('view engine', 'ejs')

app.get('/', (req, res) => {
  res.render('main', {title: "Vapomatic"})
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
