const http = require('http');
const fs = require('fs');

const querystring = require('querystring');

const { execFile } = require('child_process');

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
      res.writeHead(404, 'Resource not available.')
      res.end();
    } else if (req.url === '/effects' && req.method === 'POST') {
      handOffToApa102(req, res);
    } else {
      // serve the index.html static file
      res.writeHead(200, 'OK', { 'Content-Type', 'text/html' });
      res.end(file);
    }
  });

  server.listen(port, () => {
    console.log(`server running at port ${port}`);
  });
}

function handOffToApa102(req, res) {
  if (req.method == 'POST') {
    let fullBody = '';

    req.on('data', (chunk) => { fullBody += chunk.toString(); });

    req.on('end', function() {
      const decodedBody = querystring.parse(fullBody);
      const apaArguments = Object.keys(decodedBody).map(key => `${key}=${decodedBody[key]}`);

      execFile('apa102', ['localhost', ...apaArguments], (error, stdout, stderr) => {
        res.setHeader('Content-Type', 'text/plain');
        if (error) {
          respondError(ress, error, stderr);
        } else {
          res.writeHead(200, 'OK');
          res.end(stdout);
        }
      });
    });

    req.on('error', (error) => respondError(res, error))
  } else {
    res.writeHead(405, 'Method not supported');
    res.end();
  }
}

function respondError(res, error, additionalText) {
  console.error('[500]', error, additionalText);
  res.writeHead(500, 'Internal Server Error');
  res.end();
}
