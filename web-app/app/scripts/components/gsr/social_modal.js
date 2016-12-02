(function(){

  angular.module("polar.components.gsr")

  .service("polar.components.gsr.$Modal", ["$uibModal", function($uibModal){

    var M = function(){};

    M.open = function(user, friends, origin){
      return $uibModal.open({
        animation: true,
        templateUrl: 'app/scripts/components/gsr/social_modal_template.html',
        controller: 'polar.util.controllers.modal.Controller',
        size: 'lg',
        resolve: {
          data: function(){ return { user: user, friends: friends, origin: origin }; }
        }
      }).result;
    };

    return M;

  }]);

}());
