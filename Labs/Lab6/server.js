var fs = require('fs');
var http = require('http');
var url = require('url');
var ROOT_DIR = process.cwd();

http.createServer(function (req, res) {
	var urlObj = url.parse(req.url, true, false);
	if (urlObj.pathname.indexOf("getcity") != -1) {
		fs.readFile("cities.dat.txt", function(err, data) {

			if (err) {
				res.writeHead(404);
				res.end(JSON.stringify(err));
				return;
			}

			var reg = new RegExp('^'+urlObj.query['q']);
			var cities = data.toString().split("\n");
			var matches = [];

			cities.forEach(function(city) {
				var result = city.search(reg);
				if (result != -1) {
					matches.push({city:city});
				}
			});

			console.log(matches);
			res.writeHead(200);
			res.end(JSON.stringify(matches));

		});
	}
	else {
		if (urlObj.pathname === '/') {
			urlObj.pathname += 'html/weather.html';
		}

		fs.readFile(ROOT_DIR + urlObj.pathname, function (err,data) {
			if (err) {
				res.writeHead(404);
				res.end(JSON.stringify(err));
				return;
			}
			res.writeHead(200);
			res.end(data);
		});
	}
}).listen(80);