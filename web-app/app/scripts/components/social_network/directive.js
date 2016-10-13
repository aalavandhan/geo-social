(function(){

  var app = angular.module("polar.components.socialNetwork");

  app.directive("polarSocialNetwork", [function(){
    return{
      scope: {
        network: "="
      },
      replace: true,

      templateUrl: "app/scripts/components/social_network/template.html",
      controller: "polar.components.socialNetwork.Controller",

      link: function($scope, $element, $attributes){
        // your DOM manipulation logic for this component goes here
      }
    };
  }]);

}());
