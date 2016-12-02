(function(){

  var app = angular.module("polar.components.socialMap");
  app.controller("polar.components.socialMap.Controller",
  [ "$scope", "leafletData",
  function ($scope, leafletData){
    function init(){
      angular.extend($scope, {
        maxbounds: regions.lasVegas,
        defaults: {
          scrollWheelZoom: false,
        },
        markers: {
          default: marker
        },
        center: mapCenter
      });

      leafletData.getMap('social-map').then(function(map){
        $scope.socialMap = map;
        plotFriends($scope.friends, $scope.user);
      });
    };

    var icons = {
      query: {
        iconUrl: 'images/query-icon.png',
        iconSize: [50, 50],
      },
      user: {
        iconUrl: 'images/user-icon.ico',
        iconSize: [30, 30],
      },
      friend: {
        iconUrl: 'images/friend-icon.png',
        iconSize: [10, 10],
      }
    };

    var regions = {
      lasVegas: {
        northEast: {
          lat: 35.996499,
          lng: -114.927464,
        },
        southWest: {
          lat: 36.328529,
          lng: -115.339468,
        }
      },
    };

    var marker = {
      lat: $scope.origin.lat,
      lng: $scope.origin.lng,
      icon: icons.query
    };

    var mapCenter = {
      lat: $scope.origin.lat,
      lng: $scope.origin.lng,
      zoom: 15,
    };

    var plotFriends = function(friends, user){
      // Plot friends
      var friendMarkers = _.reduce(friends, function(m, f){
        m['f' + f.id] = {
          lat: f.geo[1],
          lng: f.geo[0],
          icon: icons.friend,
          type: 'friend',
          obj: f,
        };
        return m;
      }, { });
      angular.extend($scope.markers, friendMarkers);

      // Plot connections
      $scope.connections = _.map(friends, function(f){
        var p1 = new L.LatLng(f.geo[1], f.geo[0]);
        var p2 = new L.LatLng(user.geo[1], user.geo[0]);
        return new L.Polyline([p1,p2], {
          color: 'black',
          weight: 0.25,
          opacity: 0.5,
          smoothFactor: 1
        });
      });
      $scope.drawLines($scope.connections);

      // Plot user
      var userMarker = { };
      userMarker[user.id] = {
        lat: user.geo[1],
        lng: user.geo[0],
        icon: icons.user,
        type: 'user',
        obj: user,
      };
      angular.extend($scope.markers, userMarker);
    };

    init();
  }]);


}());
