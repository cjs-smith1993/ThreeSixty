var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
	console.log("ID " + process.pid);
  res.render('index', { title: 'Cluster', pid:process.pid });
});

router.get('/pid', function(req, res, next) {
	console.log('Service ID ' + process.pid);
	for (var i = 0; i < 10000; i++) {

	}
	res.json({
		title: 'Cluster',
		pid: process.pid
	});
});

module.exports = router;
