(function(){

  var app = angular.module("polar.components.socialNetwork");
  app.controller("polar.components.socialNetwork.Controller",[
    "$scope", "$timeout",
    "polar.util.services.$Alert", "polar.util.services.$Dialog",
    "polar.util.services.StateHandler", function ($scope, $timeout, $Alert, $Dialog, StateHandler) {

    $scope.onGraphLoad = function(graph) {
      $scope.graph = graph;
      draw();
    };

    $scope.state = StateHandler.getInstance();

    $scope.edgeMenu = [];
    $scope.nodeMenu = [];

    $scope.metadata = {};
    $scope.behavior = {};

    $scope.configration = {
      height: 600,
      width: 850,
    };

    function draw(){
      $scope.graph.clear();
      var n = angular.copy($scope.network);
      $scope.graph.data(n).redraw();
    };
  }]);

}());
