const http = require('http');
const fs = require('fs');
let file;

fs.readFile('index.html', function(err, fileContents) {
  if (err) { console.error(err); }
  file = fileContents;
});

const port = 8080;

const server = http.createServer((req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/html');
  res.end(file);
});

server.listen(port, () => {
  console.log(`node server running at port ${port}`);
});
