(function(){

  var app = angular.module("polar.components.gsr");

  app.directive("polarGsr", ["$timeout", function($timeout){
    return{
      scope: {
        types: "=",
        queryType: "=",
      },
      replace: true,

      templateUrl: "app/scripts/components/gsr/template.html",
      controller: "polar.components.gsr.Controller",

      link: function($scope, $element, $attributes){
        $scope.drawCloud = function(words){
          $("#tag-cloud").html('');

          var color = d3.scale.linear()
              .domain([0,1,2,3,4,5,6,10,15,20,100])
              .range(["#ddd", "#ccc", "#bbb", "#aaa", "#999", "#888", "#777", "#666", "#555", "#444", "#333", "#222"]);

          d3.layout.cloud().size([$($element).width() * 0.90, 600 * 0.90])
            .words(words)
            .rotate(0)
            .fontSize(function(d) { return d.size; })
            .on("end", draw)
            .start();


          function draw(){
            d3.select("#tag-cloud").append("svg")
              .attr("width", $($element).width())
              .attr("height", 600)
              .attr("class", "wordcloud")
              .append("g")
              // without the transform, words words would get cutoff to the left and top, they would
              // appear outside of the SVG area
              .attr("transform", "translate(320,200)")
              .selectAll("text")
              .data(words)
              .enter().append("text")
              .style("font-size", function(d) { return d.size + "px"; })
              .style("fill", function(d, i) { return color(i); })
              .attr("transform", function(d) {
                  return "translate(" + [d.x, d.y] + ")rotate(" + d.rotate + ")";
              })
              .text(function(d) { return d.text; });
          };
        };

        // your DOM manipulation logic for this component goes here
        $scope.hideCircle = function(type){
          if($scope[type]){
            $scope.map.removeLayer($scope[type]);
          };
        };

        $scope.drawCircle = function(pos, radius, type){
          $timeout(function(){
            $scope.hideCircle(type);
            var drawnItems = new L.FeatureGroup();


            var circle = new L.Circle(pos, radius * 1000,{
              opacity: 0.3,
              color: type == 'range' ? 'grey' : 'red',
            });

            circle.addTo(drawnItems);
            $scope[type] = drawnItems;
            $scope.map.addLayer(drawnItems);
          });
        };


        $scope.drawLines = function(polyLines){
          $timeout(function(){
            _.each(polyLines, function(l){
              $scope.map.addLayer(l);
            });
          });
        };

        $scope.removeObjects = function(objs){
          $timeout(function(){
            _.each(objs, function(l){
              $scope.map.removeLayer(l);
            });
          });
        };
      }
    };
  }]);

}());
