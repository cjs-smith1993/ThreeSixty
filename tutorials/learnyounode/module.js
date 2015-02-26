module.exports = function(directory, filter, callback) {
	var fs = require('fs');
	var path = require('path');

	fs.readdir(directory, function(err, list) {
		if (err) {
			callback(err);
		}
		else {
			var files = [];
			list.filter(function(file) {
				return path.extname(file) == '.'+filter;
			}).forEach(function(file) {
				files.push(file);
			});
			callback(null, files);
		}
	});
};