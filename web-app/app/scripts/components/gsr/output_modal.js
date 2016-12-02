(function(){

  angular.module("polar.components.gsr")

  .service("polar.components.gsr.$OPModal", ["$uibModal", function($uibModal){

    var M = function(){};

    M.open = function(o){
      return $uibModal.open({
        animation: true,
        templateUrl: 'app/scripts/components/gsr/output_modal_template.html',
        controller: 'polar.util.controllers.modal.Controller',
        size: 'lg',
        resolve: {
          data: function(){ return { output: o }; }
        }
      }).result;
    };

    return M;

  }]);

}());
