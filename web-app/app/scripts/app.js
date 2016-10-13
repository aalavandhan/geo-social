'use strict';

/**
 * @ngdoc overview
 * @name polar
 * @description
 * # polar
 *
 * Main module of the application.
 */
angular
.module('polar', [
  'ngAnimate',
  'ngCookies',
  'ngResource',
  'ngRoute',
  'ngSanitize',
  'ngTouch',
  'LocalStorageModule',
  'polar.components',
  'polar.data',
  'polar.sections',
  'polar.util',
  "ui.bootstrap",
])

.config(["$routeProvider", function ($routeProvider){
  $routeProvider
    .when('/index', {
      templateUrl: 'app/scripts/sections/home/index.html',
      controller: 'polar.util.controllers.StaticPageController'
    })

    .when('/api_doc', {
      templateUrl: 'app/scripts/sections/home/api_doc.html',
      controller: 'polar.util.controllers.StaticPageController'
    })

    .when('/gsr', {
      templateUrl: 'app/scripts/sections/query/gsr.html',
      controller: 'polar.util.controllers.StaticPageController'
    })

    .otherwise({
      redirectTo: '/index'
    });
}]);
