const express = require('express')

const fs = require("fs")

const app = express()
const host = "127.0.0.1"
const port = 8080

// Cópias dos últimos gráficos lidos corretamente
var graph = {
  temp: '',
  gist: ''
}

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
  console.log('HTTP server closed')
})
