var http = require('http');
var bl = require('bl');

var URLs = process.argv.slice(2);
var ret = [];
var numDone = 0;

URLs.forEach(function(url, i) {
	http.get(url, function(response) {
		response.setEncoding('utf8');
		response.pipe(bl(function(err, data) {
			ret[i] = data.toString();
			numDone++;
			if (numDone == 3) {
				ret.forEach(function(str) {
					console.log(str);
				});
			}
		}));
	});
});