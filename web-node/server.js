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
    if (false) {
      // serve a favicon
    } else if (false) {
      // do the apa102 call
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
