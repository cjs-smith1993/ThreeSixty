var http = require('http');
var url = require('url');
var map = require('through2-map');
var port = process.argv[2];

var server = http.createServer(function(request, response) {
	var req = url.parse(request.url, true);

	console.log(req);

	var ret = {};


	if (req.pathname == '/api/parsetime') {
		var time = new Date(req.query.iso);
		ret['hour'] = time.getHours();
		ret['minute'] = time.getMinutes();
		ret['second'] = time.getSeconds();
	}
	else if (req.pathname == '/api/unixtime') {
		var date = new Date(req.query.iso);
		var time = date.getTime();
		ret['unixtime'] = time;

		console.log(time)
	}

	var json = JSON.stringify(ret);
	// console.log(json);
	response.writeHead(200, {'Content-Type': 'application/json'});
	response.write(json);
	response.end();
});

server.listen(port);