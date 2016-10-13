(function(){

  var app = angular.module("polar.components.gsr");
  app.controller("polar.components.gsr.Controller",
  [ "$scope", "$timeout", "leafletData", "polar.data.Query", "polar.util.services.StateHandler", "polar.components.gsr.$Modal",
  function ($scope, $timeout, leafletData, Query, StateHandler, $Modal){
    function init(){
      angular.extend($scope, {
        maxbounds: regions.austin,
        defaults: {
          scrollWheelZoom: false,
        },
        markers: {
          default: marker
        },
        center: mapCenter,
        events: {
          markers:{
            enable: [ 'dragend' ]
          }
        }
      });
      $scope.radius = 4;
      $scope.K      = 1;

      $scope.state = StateHandler.getInstance();
    };

    leafletData.getMap().then(function(map){
      $scope.map = map;
    });

    var regions = {
      austin: {
        northEast: {
          lat: 30.4098181886,
          lng: -97.5564050674
        },
        southWest: {
          lat: 30.1313942671,
          lng: -97.8997278214
        }
      },
    };

    var marker = {
      lat: 30.2021415,
      lng: -97.66682625,
      message: "Drag me to adjust the query location",
      focus: true,
      draggable: true,
    };

    var mapCenter = {
      lat: 30.2021415,
      lng: -97.66682625,
      zoom: 13,
    };

    init();

    $scope.$on("leafletDirectiveMarker.dragend", function(event, args){
      $scope.markers[args.modelName].lat = args.model.lat;
      $scope.markers[args.modelName].lng = args.model.lng;
      $scope.drawCircle([ $scope.markers.default.lat, $scope.markers.default.lng ], $scope.radius);
    });

    $scope.$watch("radius", function(nv){
      if(nv){
        $scope.drawCircle([ $scope.markers.default.lat, $scope.markers.default.lng ], $scope.radius);
      };
    });

    $scope.query = function(){
      $scope.state.initiate();

      new Query().$gsr({
        radius: $scope.radius,
        K: $scope.K,
        lat: $scope.markers.default.lat,
        lon: $scope.markers.default.lng
      }).then(function(r){
        $scope.results = r.response;
        $scope.state.success();
      }, function(){
        $scope.state.fatal();
      })
    };

    var networkify = function(d, s){
      var edges = _.map(d, function(e){
        return { out: e[0], in: s, type:"friend of" }
      });

      var nodes = _.map(d, function(e){
        return { id: e[0], type: "user" }
      });
      nodes.push({ id: s, type: "top-k-user" })

      return nodes.concat(edges);
    };

    $scope.viewNetwork = function(e){
      $Modal.open(networkify(e.friends, e.userId));
    };

  }]);

}());
