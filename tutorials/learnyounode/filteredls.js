var fs = require('fs');
var dir = process.argv[2];
var filter = '.'+process.argv[3];

var path = require('path');
fs.readdir(dir, function(err, list) {
	list.filter(function(file) {
		return path.extname(file) == filter;
	}).forEach(function(file) {
		console.log(file);
	})
});