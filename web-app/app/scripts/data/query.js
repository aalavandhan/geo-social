(function(){
  var app = angular.module("polar.data");
  app.factory("polar.data.Query",
    ["$resource", "$q",

    function($resource, $q){

      var Resource = $resource("/api", { },{
        gsr: {
          method: 'GET',
          url: "/api/gsr",
        }
      }),
      proto = Resource.prototype;

      return Resource;
    }
  ]);

}());
