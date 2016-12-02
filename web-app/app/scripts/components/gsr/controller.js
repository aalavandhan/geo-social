(function(){

  var app = angular.module("polar.components.gsr");
  app.controller("polar.components.gsr.Controller",
  [ "$scope", "$timeout", "leafletData", "polar.data.Query", "polar.util.services.StateHandler", "polar.components.gsr.$Modal", "polar.components.gsr.$OPModal",
  function ($scope, $timeout, leafletData, Query, StateHandler, $Modal, $OPModal){
    function init(){
      angular.extend($scope, {
        maxbounds: regions.lasVegas,
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

      $scope.params = {
        radius: 2,
        K: 2,
        W: 0.5,
        M: 5,
        BPower: 3,
        keywords: [ ],
      };

      $scope.qType = $scope.types[0];

      $scope.state = StateHandler.getInstance();
    };

    leafletData.getMap().then(function(map){
      $scope.map = map;
    });

    var icons = {
      query: {
        iconUrl: 'images/query-icon.png',
        iconSize: [38, 40],
      },
      place: {
        iconUrl: 'images/place-icon.png',
        iconSize: [20, 20],
      },
      user: {
        iconUrl: 'images/user-icon.ico',
        iconSize: [30, 30],
      },
      friend: {
        iconUrl: 'images/friend-icon.png',
        iconSize: [5,5],
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
      lat: 36.121360552785,
      lng: -115.17404204278,
      // message: "Drag me to adjust the query location",
      focus: true,
      draggable: true,
      icon: icons.query
    };

    var mapCenter = {
      lat: 36.127307,
      lng: -115.174568,
      zoom: 13,
    };

    Query.clusters().$promise.then(function(d){
      $scope.clusters = d;
    });

    init();

    $scope.$on("leafletDirectiveMarker.dragend", function(event, args){
      $scope.markers[args.modelName].lat = args.model.lat;
      $scope.markers[args.modelName].lng = args.model.lng;
      drawRadius();
    });

    $scope.$watch("params.radius", function(nv){
      if(nv){
        drawRadius();
      };
    });

    $scope.query = function(){
      $scope.state.initiate();
      new Query()["$" + $scope.queryType]({
        radius: $scope.params.radius,
        K: $scope.params.K,
        lat: $scope.markers.default.lat,
        lon: $scope.markers.default.lng,
        qType: $scope.qType,
        W: $scope.params.W,
        M: $scope.params.M,
        BPower: $scope.params.BPower,
        keywords: _.pluck($scope.params.keywords, "text"),
      }).then(function(r){
        hideMap();
        if($scope.queryType == 'gsr'){
          $scope.users = r.users;
          drawUserMap($scope.users);
        } else if($scope.queryType == 'gsk'){
          $scope.keywords = r.keywords;
          var kWord = _.map(r.keywords, function(k){
            return {
              text: k.keyword,
              score: k.tf
              //k.tf * k.idf
            }
          });
          var maxScore = _.max(kWord, function(k){
            return k.score
          }).score;
          var nkWord = _.map(kWord, function(k){
            k.size = (k.score / maxScore) * 50;
            return k;
          });
          $scope.drawCloud(nkWord);
        } else if($scope.queryType == "gsp"){
          $scope.places = r.places;
          drawPlaceMap($scope.places);
        };

        $scope.output = r.output;
        $OPModal.open($scope.output);

        $scope.state.success();
      }, function(){
        $scope.state.fatal();
      })
    };

    var networkify = function(d, s){
      var edges = _.map(d, function(e){
        return { out: e.userId, in: s, type:"friend of" }
      });
      var nodes = _.map(d, function(e){
        return { id: e.userId, type: "user" }
      });
      nodes.push({ id: s, type: "top-k-user" })
      return nodes.concat(edges);
    };

    var drawUserMap = function(users){
      var userMarkers = _.reduce(users, function(m, u){
        m[u.id] = {
          lat: u.geo[1],
          lng: u.geo[0],
          icon: icons.user,
          type: 'user',
          obj: u,
          message: "<table class='table table-striped'><tbody><tr><th>ID</th><td>" + u.id +"</td></tr><tr><th>Distance</th><td>" + u.uDist +" km</td></tr><tr><th>Number of friends</th><td>" + u.nFriends +"</td></tr></tbody></table>",
        };
        return m;
      }, { });
      angular.extend($scope.markers, userMarkers);
      var farthest = _.max(users, function(u){ return u.uDist });
      $scope.drawCircle([ $scope.markers.default.lat, $scope.markers.default.lng ], farthest.uDist, 'bounding');
    };

    var drawPlaceMap = function(places){
      var placeMarkers = _.reduce(places, function(m, f){
        m[f.id] = {
          lat: f.geo[1],
          lng: f.geo[0],
          icon: icons.place,
          type: 'place',
          obj: f,
          message: "<table class='table table-striped'><tbody><tr><th>ID</th><td>" + f.id +"</td></tr><tr><th>Distance</th><td>" + f.dist +" km</td></tr></table>",
        };
        return m;
      }, { });
      angular.extend($scope.markers, placeMarkers);
    };

    var hideMap = function(){
      _.each($scope.users, function(u){
        delete $scope.markers[u.id];
      });
      _.each($scope.places, function(p){
        delete $scope.markers[p.id];
      });
      $scope.hideCircle('bounding');
    };

    var drawRadius = function(){
      if($scope.qType == 'LCA_UD' || $scope.qType == 'HGS_UD' || $scope.qType == 'GSK_FSKR'){
        $scope.drawCircle([ $scope.markers.default.lat, $scope.markers.default.lng ], $scope.params.radius, 'range');
      } else {
        $scope.hideCircle('range');
      };
    };

    $scope.viewNetwork = function(e){
      $Modal.open(e, e.friends, $scope.markers.default);
    };

    $scope.$watch("qType", function(nv){
      drawRadius();
      $scope.users    = [ ];
      $scope.places   = [ ];
      $scope.keywords = [ ];
    });

  }]);

}());
