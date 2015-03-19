var express = require('express');
var bodyParser = require('body-parser');
var basicAuth = require('basic-auth-connect');
var https = require('https');
var http = require('http');
var fs = require('fs');
var url = require('url');
var app = express();
app.use(bodyParser());
var options = {
	host: '127.0.0.1',
	key: fs.readFileSync('ssl/server.key'),
	cert: fs.readFileSync('ssl/server.crt')
};

http.createServer(app).listen(80);
https.createServer(options, app).listen(443);

var auth = basicAuth(function(user, pass) {
	return user === 'cs360' && pass === 'test';
})

app.get('/', function (req, res) {
	res.send("Get Index");
});
app.use('/', express.static('./html', {maxAge: 60*60*1000}));
app.get('/getcity', getCity);
app.get('/comment', getComment);
app.post('/comment', auth, postComment);

function getCity(req, res) {
	console.log('In getcity route');
	var urlObj = url.parse(req.url, true, false);
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
};

function getComment(req, res) {
	console.log('In comment route');

	// Read all of the database entries and return them in a JSON array
	var MongoClient = require('mongodb').MongoClient;
	MongoClient.connect("mongodb://localhost/weather", function(err, db) {
		if(err) throw err;
		db.collection("comments", function(err, comments){
			if(err) throw err;
			comments.find(function(err, items){
				items.toArray(function(err, itemArr){
					console.log("Document Array: ");
					console.log(itemArr);
					res.writeHead(200);
					res.end(JSON.stringify(itemArr));
				});
			});
		});
	});
};

function postComment(req, res) {
	console.log('In POST comment route');

	var name = req.body.Name;
	var comment = req.body.Comment;

	// Now put it into the database
	var MongoClient = require('mongodb').MongoClient;
	MongoClient.connect('mongodb://localhost/weather', function(err, db) {
		if(err) throw err;
		if (name === 'clear') {
			console.log('clearing');
			db.collection('comments', function(err, collection) {
				collection.remove({}, function(err, result) {

				});
			});
		}
		else {
			db.collection('comments').insert(req.body,function(err, records) {
				console.log('Record added as '+records[0]._id);
			});
		}
	});

	res.writeHead(200);
	res.end('');
};