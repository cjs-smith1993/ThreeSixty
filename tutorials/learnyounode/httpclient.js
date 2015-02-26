var URL = process.argv[2];
var http = require('http');
http.get(URL, function(response) {
	response.setEncoding('utf8');
	response.on("data", function(data) {
		console.log(data);
	});
})