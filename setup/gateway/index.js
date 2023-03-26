const express = require('express')

const fs = require("fs")

const app = express()
const host = "127.0.0.1"
const port = 8080

var temp_prev
var gist_prev

app.get('/', (req, res) => {
  fs.readFile('../setup.html', 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      return;
    }
    res.send(data);
    return;
  });
})

app.get('/temp.svg', (req, res) => {
  var stats = fs.statSync('../temp.svg')
  if ( stats.size > 0 )

    fs.readFile('../temp.svg', 'utf8', (err, data) => {
      if (err) {
        console.error(err);
        return;
      }
      temp_prev = data
      res.setHeader("Content-Type", "image/svg+xml")
      res.send(data);
      return;
    });

  else {
    res.setHeader("Content-Type", "image/svg+xml")
    res.send(temp_prev);
  }

})

app.get('/gist.svg', (req, res) => {
  var stats = fs.statSync('../gist.svg')
  if ( stats.size > 0 )

    fs.readFile('../gist.svg', 'utf8', (err, data) => {
      if (err) {
        console.error(err);
        return;
      }
      gist_prev = data
      res.setHeader("Content-Type", "image/svg+xml")
      res.send(data);
      return;
    });

  else {
    res.setHeader("Content-Type", "image/svg+xml")
    res.send(gist_prev);
  }

})

app.listen(port, host, () => {
  console.log(`Server ready at http://${host}:${port}`)
})
