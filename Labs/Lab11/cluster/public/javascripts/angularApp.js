angular.module('clusterApp', [])
	.controller('MainCtrl', [
	'$scope', '$http',
	function($scope, $http) {
		$scope.cluster = [{
			pid:1234
		}];

		$scope.getPIDs = function() {
			$scope.cluster = [{
				pid:12
			}, {
				pid:34
			}];
		};

		$scope.getMyPIDs = function() {
			for (var i = 0; i < 100; i++) {
				$http.get('/pid').success(function(data) {
					console.log('getAll');
					console.log(data);
					$scope.cluster.push(data);
				});
			}
		};
	}

]);
