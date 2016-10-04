const http = require('http');
const fs = require('fs');

const port = 8080;

fs.readFile('index.html', function(err, file) {
  if (err) {
    console.error(err);
  } else {
    startServer(file);
  }
});

function startServer(file) {
  const server = http.createServer((req, res) => {
    if (req.url === '/favicon.ico' && req.method === 'GET') {
      res.statusCode = 404;
      res.end();
    } else if (false && req.url === '' && req.method === 'POST') {

    } else {
      // serve the index.html static file
      res.statusCode = 200;
      res.setHeader('Content-Type', 'text/html');
      res.end(file);
    }
  });

  server.listen(port, () => {
    console.log(`server running at port ${port}`);
  });
}
