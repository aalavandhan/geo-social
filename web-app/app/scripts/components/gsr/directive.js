(function(){

  var app = angular.module("polar.components.gsr");

  app.directive("polarGsr", ["$timeout", function($timeout){
    return{
      scope: { },
      replace: true,

      templateUrl: "app/scripts/components/gsr/template.html",
      controller: "polar.components.gsr.Controller",

      link: function($scope, $element, $attributes){
        // your DOM manipulation logic for this component goes here

        $scope.drawCircle = function(pos, radius){
          $timeout(function(){
            if($scope.drawnItems){
              $scope.map.removeLayer($scope.drawnItems);
            };
            var drawnItems = new L.FeatureGroup();
            var circle = new L.Circle(pos, radius * 1000);
            circle.addTo(drawnItems);
            $scope.drawnItems = drawnItems;
            $scope.map.addLayer(drawnItems);
          });
        };
      }
    };
  }]);

}());
