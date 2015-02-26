var URL = process.argv[2];
var http = require('http');
var bl = require('bl');
http.get(URL, function(response) {
	var ret = "";

	response.setEncoding('utf8');
	response.pipe(bl(function(err, data) {
		ret += data.toString();
	}));
	response.on('end', function() {
		console.log(ret.length);
		console.log(ret);
	});
})