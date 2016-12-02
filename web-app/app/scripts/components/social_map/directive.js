(function(){

  var app = angular.module("polar.components.socialMap");

  app.directive("polarSocialMap", ["$timeout", function($timeout){
    return{
      scope: {
        user: "=",
        friends: "=",
        origin: "=",
      },
      replace: true,

      templateUrl: "app/scripts/components/social_map/template.html",
      controller: "polar.components.socialMap.Controller",

      link: function($scope, $element, $attributes){
        // your DOM manipulation logic for this component goes here

        $scope.drawLines = function(polyLines){
          $timeout(function(){
            _.each(polyLines, function(l){
              $scope.socialMap.addLayer(l);
            });
          });
        };

        $scope.removeObjects = function(objs){
          $timeout(function(){
            _.each(objs, function(l){
              $scope.socialMap.removeLayer(l);
            });
          });
        };

      }
    };
  }]);

}());
