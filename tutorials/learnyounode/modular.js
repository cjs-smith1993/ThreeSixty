var dir = process.argv[2];
var filter = process.argv[3];

var mymodule = require('./module');
mymodule(dir, filter, function(err, data) {
	if (err) {
		console.log(err);
	}
	else {
		data.forEach(function(file) {
			console.log(file);
		})
	}
});