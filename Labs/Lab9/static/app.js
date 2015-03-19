angular.module('weatherNews', ['ui.router'])
.factory('postFactory', [function() {
	var o = {
		posts: []
	};
	return o;
}])
.config([
	'$stateProvider',
	'$urlRouterProvider',
	function($stateProvider, $urlRouterProvider) {
		$stateProvider.state('home', {
			url: '/home',
			templateUrl: '/home.html',
			controller: 'MainCtrl'
		}).state('posts', {
			url: '/posts/{id}',
			templateUrl: '/posts.html',
			controller: 'PostCtrl'
		});
		$urlRouterProvider.otherwise('home');
	}
])
.controller('MainCtrl', [
	'$scope',
	'postFactory',
	function($scope, postFactory){
		$scope.posts = postFactory.posts;

		$scope.addPost = function() {
			$scope.posts.push({
				title: $scope.formContent,
				upvotes: 0
			});
			$scope.formContent = '';
		}

		$scope.incrementUpvotes = function(post) {
			post.upvotes++;
		}
	}
])
.controller('PostCtrl', [
	'$scope',
	'$stateParams',
	'postFactory',
	function($scope, $stateParams, postFactory) {

		console.log($stateParams);
		console.log(postFactory.posts);

		$scope.post = postFactory.posts[$stateParams.id];
		$scope.post.comments = [];

		$scope.addComment = function() {
			if ($scope.body === '') {
				return;
			}
			console.log($scope.post);
			$scope.post.comments.push({
				body: $scope.body,
				upvotes: 0
			});
			$scope.body = '';
		};
		$scope.incrementUpvotes = function(comment) {
			comment.upvotes++;
		}
	}
]);
