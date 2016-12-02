(function(){
  var app = angular.module("polar.data");
  app.factory("polar.data.Query",
    ["$resource", "$q",

    function($resource, $q){

      var Resource = $resource("/api", { },{
        gsp: {
          method: 'GET',
          url: "/api/gsq",
          cache: true,
          interceptor: {
           response:function(data, status){
             var op = data.data.response.output.split('--------------------------------------');
             var d = op[1].split("\n");
             var output = op[0] + op[2];

             var parse = function(line){
              var pData = line.split(",");
              return {
                id: parseFloat(pData[0]),
                vnp: parseFloat(pData[1]),
                tfidf: parseFloat(pData[2]),
                distScore: parseFloat(pData[3]),
                dist: parseFloat(pData[4]),
                geo:  [ parseFloat(pData[5]), parseFloat(pData[6]) ],
              };
             };

             var places = _.reduce(d, function(m,l){
              if(l == ""){
                return m;
              };

              m.push( parse(l) );

              return m;
             }, [ ]);

             return {
              output: output,
              places: places,
             };
           }
         }
        },
        gsk: {
          method: 'GET',
          url: "/api/gsq",
          cache: true,
          interceptor: {
           response:function(data, status){
             var op = data.data.response.output.split('--------------------------------------');
             var d = op[1].split("\n");
             var output = op[0] + op[2];

             var parse = function(line){
              var kData = line.split(",");
              return {
                rank: parseFloat(kData[0]),
                keyword: kData[1],
                idf: parseFloat(kData[2]),
                tf: parseFloat(kData[3]),
              };
             };

             var keywords = _.reduce(d, function(m,l){
              if(l == ""){
                return m;
              };

              m.push( parse(l) );

              return m;
             }, [ ]);

             return {
              output: output,
              keywords: keywords,
             };
           }
         }
        },
        gsr: {
          method: 'GET',
          url: "/api/gsq",
          cache: true,
          interceptor: {
           response:function(data, status){
             var op = data.data.response.output.split('--------------------------------------');
             var d = op[1].split("\n");
             var output = op[0] + op[2];

             var parseUser = function(uLine){
              var uData = uLine.split(",");
              return {
                id: parseFloat(uData[0]),
                uDist: parseFloat(uData[1]),
                geo:  [ parseFloat(uData[2]), parseFloat(uData[3]) ],
                aDist:  parseFloat(uData[4]),
                maxDist:  parseFloat(uData[5]),
                score:  parseFloat(uData[6]),
                nFriends: parseFloat(uData[7]),
                friends: [ ],
              };
             };

             var parseFriend = function(fLine){
              var fData = fLine.split(",");
              return {
                id: fData[0],
                fDist: parseFloat(fData[1]),
                geo:  [ parseFloat(fData[3]), parseFloat(fData[4]) ],
              };
             };

             var users = _.reduce(d, function(m,l){
              if(l == ""){
                return m;
              };

              if(l.split(",").length == 8){
                m.push( parseUser(l) );
              } else {
                m[ m.length - 1 ].friends.push( parseFriend(l) );
              };
              return m;
             }, [ ]);

             return {
              output: output,
              users: users,
             };

           }
         }
        },

        clusters: {
          method: 'GET',
          url: "/data/clustering.txt",
          cache: true,
          interceptor: {
           response:function(data, status){
             var d = data.data.split("\n");

             var clusters = _.reduce(d, function(m, r){
                var x = r.split(" ");
                var y = {
                  cluster: parseInt(x[0]),
                  id: parseInt(x[1]),
                };
                m[y.id] = y.cluster
                return m;
             }, { });

             return clusters;
           }
         }
        },
      }),
      proto = Resource.prototype;

      return Resource;
    }
  ]);

}());
