angular.module('polar').run(['$templateCache', function($templateCache) {
  'use strict';

  $templateCache.put('app/scripts/components/flash/template.html',
    "<div>\n" +
    "  <div class=\"c-alert c-{{flashConfig.type || 'info'}}\" data-ng-if=\"(flashConfig && !closeFlash)\">\n" +
    "    <span class=\"glyphicon c-{{flashConfig.type || 'info'}}-icon\"></span>\n" +
    "    <span class=\"body\" data-ng-bind=\"flashConfig.message\"></span>\n" +
    "    <button type=\"button\" class=\"close\" data-ng-click=\"close()\" aria-hidden=\"true\">&times;</button>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/footer/template.html',
    "<div class=\"footer\">\n" +
    "  <span class=\"pull-right\">\n" +
    "    A CSCI-587 Project\n" +
    "  </span>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/gsr/output_modal_template.html',
    "<div>\n" +
    "  <div class=\"modal-body\">\n" +
    "    <pre data-ng-bind=\"data.output\"></pre>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/gsr/social_modal_template.html',
    "<div>\n" +
    "  <div class=\"modal-body\">\n" +
    "    <div polar-social-map data-user=\"data.user\" data-friends=\"data.friends\" data-origin=\"data.origin\"></div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/gsr/template.html',
    "<div>\n" +
    "  <div class=\"row\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "\n" +
    "      <leaflet\n" +
    "        width=\"100%\"\n" +
    "        height=\"680px\"\n" +
    "        maxbounds=\"maxbounds\"\n" +
    "        defaults=\"defaults\"\n" +
    "        markers=\"markers\"\n" +
    "        center=\"center\"></leaflet>\n" +
    "\n" +
    "    </div>\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"row\">\n" +
    "    <div class=\"col-md-10\">\n" +
    "      <div class=\"row\">\n" +
    "        <div class=\"col-md-4\">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>Query Type</label>\n" +
    "            <select ng-model=\"qType\" ng-options=\"t for t in types\" class=\"form-control\"></select>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "        <div class=\"col-md-4\" data-ng-if=\"qType == 'LCA_UD' || qType == 'HGS_UD' || qType == 'GSK_FSKR' \">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>Search Radius</label>\n" +
    "            <rzslider rz-slider-model=\"params.radius\" rz-slider-options=\"{ 'floor': 1, 'ceil': 10.0, 'step': 0.1, 'precision': 1 }\"></rzslider>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "\n" +
    "        <div class=\"col-md-4\"  data-ng-if=\"qType == 'LCA_UD' || qType == 'LCA_DD' || qType == 'HGS_UD' || qType == 'HGS_DD' || qType == 'NSG_LAZY' || qType == 'GST_LAZY' || qType == 'GST_EAGER' || qType == 'GSK_NPRU' || qType == 'GSK_NSTP' || qType == 'GSK_FSKR'\n" +
    "        || qType == 'RCA'\">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>Top-K</label>\n" +
    "            <rzslider rz-slider-model=\"params.K\" rz-slider-options=\"{ 'floor': 1, 'ceil': 100 } \"></rzslider>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "        <div class=\"col-md-4\" data-ng-if=\"qType == 'RCA' || qType == 'LCA_UD' || qType == 'LCA_DD'\">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>W</label>\n" +
    "            <rzslider rz-slider-model=\"params.W\" rz-slider-options=\"{ 'floor': 0, 'ceil': 1.0, 'step': 0.01, 'precision': 1 } \"></rzslider>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "        <div class=\"col-md-4\" data-ng-if=\"qType == 'NSG_LAZY'\">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>M</label>\n" +
    "            <rzslider rz-slider-model=\"params.M\" rz-slider-options=\"{ 'floor': 0, 'ceil': 10 } \"></rzslider>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "        <div class=\"col-md-4\" data-ng-if=\"qType == 'LCA_DD' || qType == 'HGS_DD'\">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>y = k<sup>x</sup></label>\n" +
    "            <rzslider rz-slider-model=\"params.BPower\" rz-slider-options=\"{ 'floor': 0, 'ceil': 10.0, 'step': 0.1, 'precision': 1 } \"></rzslider>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "        <div class=\"col-md-4\"  data-ng-if=\"qType == 'GSK_NSTP' || qType == 'GSK_NPRU'\">\n" +
    "          <div class=\"form-group\">\n" +
    "            <label>Key Words</label>\n" +
    "            <tags-input ng-model=\"params.keywords\"></tags-input>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "      </div>\n" +
    "    </div>\n" +
    "\n" +
    "    <div class=\"col-md-2\">\n" +
    "      <div class=\"form-group\">\n" +
    "        <label>&nbsp;</label>\n" +
    "        <button class=\"btn btn-success btn-block\"\n" +
    "                data-ng-click=\"query()\"\n" +
    "                data-ng-disabled=\"state.isWorking\">Query</button>\n" +
    "      </div>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "\n" +
    "\n" +
    "  <div class=\"row\" data-ng-show=\"users.length > 0\">\n" +
    "    <div class=\"col-md-8\">\n" +
    "      <h3>Top User Group</h3>\n" +
    "      <table class=\"table\">\n" +
    "        <thead>\n" +
    "          <tr>\n" +
    "            <th>User</th>\n" +
    "            <th>Distance (KM)</th>\n" +
    "            <th>Score</th>\n" +
    "            <th>Cluster</th>\n" +
    "            <th data-ng-if=\"qType != 'GSK_NPRU'\">Nearby Friends</th>\n" +
    "          </tr>\n" +
    "        </thead>\n" +
    "        <tbody>\n" +
    "          <tr data-ng-repeat=\"r in users\">\n" +
    "            <td data-ng-bind=\"r.id\"></td>\n" +
    "            <td data-ng-bind=\"r.uDist\"></td>\n" +
    "            <td data-ng-bind=\"r.score\"></td>\n" +
    "            <td data-ng-bind=\"clusters[r.id]\"></td>\n" +
    "            <td data-ng-if=\"qType != 'GSK_NPRU'\"><a data-ng-bind=\"r.nFriends\" data-ng-click=\"viewNetwork(r)\"></a></td>\n" +
    "          </tr>\n" +
    "        </tbody>\n" +
    "      </table>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"row\" data-ng-show=\"places.length > 0\">\n" +
    "    <div class=\"col-md-8\">\n" +
    "      <h3>Top Points of interest</h3>\n" +
    "      <table class=\"table\">\n" +
    "        <thead>\n" +
    "          <tr>\n" +
    "            <th>Place</th>\n" +
    "            <th>Distance (KM)</th>\n" +
    "            <th>TF-IDF</th>\n" +
    "            <th>VNP</th>\n" +
    "            <th>Dist Score</th>\n" +
    "          </tr>\n" +
    "        </thead>\n" +
    "        <tbody>\n" +
    "          <tr data-ng-repeat=\"r in places\">\n" +
    "            <td data-ng-bind=\"r.id\"></td>\n" +
    "            <td data-ng-bind=\"r.dist\"></td>\n" +
    "            <td data-ng-bind=\"r.tfidf\"></td>\n" +
    "            <td data-ng-bind=\"r.vnp\"></td>\n" +
    "            <td data-ng-bind=\"r.distScore\"></td>\n" +
    "          </tr>\n" +
    "        </tbody>\n" +
    "      </table>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "\n" +
    "\n" +
    "  <div class=\"row\" data-ng-show=\"keywords.length > 0\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "      <div id=\"tag-cloud\"></div>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/navigation/template.html',
    "<nav class=\"navbar navbar-default\">\n" +
    "  <div class=\"container\">\n" +
    "    <div class=\"navbar-header\">\n" +
    "      <button type=\"button\" class=\"navbar-toggle collapsed\" data-toggle=\"collapse\" data-target=\"#navbarLinks\">\n" +
    "        <span class=\"sr-only\">Toggle navigation</span>\n" +
    "        <span class=\"icon-bar\"></span>\n" +
    "        <span class=\"icon-bar\"></span>\n" +
    "        <span class=\"icon-bar\"></span>\n" +
    "      </button>\n" +
    "      <a class=\"navbar-brand\" href=\"#/\">Geo Social Query Interface</a>\n" +
    "    </div>\n" +
    "\n" +
    "    <div class=\"collapse navbar-collapse navbar-collapse\" id=\"navbarLinks\">\n" +
    "      <ul class=\"nav navbar-nav\">\n" +
    "        <li>\n" +
    "          <div style=\"padding-top: 5px;\">\n" +
    "            <select class=\"form-control\"><option>4-Square Las Vegas</option></select>\n" +
    "          </div>\n" +
    "        </li>\n" +
    "      </ul>\n" +
    "\n" +
    "      <ul class=\"nav navbar-nav navbar-right\">\n" +
    "        <li data-ng-class=\"$location.path() == '/gsr' ? 'active' : '' \"><a href=\"#/gsr\">User Groups</a></li>\n" +
    "        <li data-ng-class=\"$location.path() == '/gsp' ? 'active' : '' \"><a href=\"#/gsp\">Points of Interest</a></li>\n" +
    "        <li data-ng-class=\"$location.path() == '/gsk' ? 'active' : '' \"><a href=\"#/gsk\">Keywords</a></li>\n" +
    "      </ul>\n" +
    "    </div>\n" +
    "\n" +
    "  </div>\n" +
    "</nav>\n"
  );


  $templateCache.put('app/scripts/components/social_map/template.html',
    "<div>\n" +
    "  <div class=\"row\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "      <leaflet\n" +
    "        id=\"social-map\"\n" +
    "        width=\"100%\"\n" +
    "        height=\"680px\"\n" +
    "        maxbounds=\"maxbounds\"\n" +
    "        defaults=\"defaults\"\n" +
    "        markers=\"markers\"\n" +
    "        center=\"center\"></leaflet>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/social_network/template.html',
    "<div>\n" +
    "  <div class=\"row\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "      <div class=\"graph-container\"\n" +
    "           tg-graph\n" +
    "           graph=\"graph\"\n" +
    "           on-load=\"onGraphLoad(graph)\"\n" +
    "           edge-menu=\"edgeMenu\"\n" +
    "           node-menu=\"nodeMenu\"\n" +
    "           metadata=\"metadata\"\n" +
    "           configration=\"configration\"\n" +
    "           behavior=\"behavior\"\n" +
    "           stream=\"stream\"\n" +
    "           helpers=\"helpers\"></div>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/components/graph/aside/template.html',
    "<div class=\"aside\" data-ng-class=\"show ? 'show' : ''\" style=\"z-index: 10000;\">\n" +
    "  <div class=\"aside-header\">\n" +
    "    <button class=\"close\" ng-click=\"close()\" type=\"button\">×</button>\n" +
    "    <h5 class=\"aside-title\" data-ng-bind=\"element.id\"></h5>\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"row\">\n" +
    "    <ul class=\"nav nav-pills nav-justified nav-pills-aside\">\n" +
    "      <li class=\"active\"><a href=\"#\"><i class=\"fa fa-bars\"></i></a></li>\n" +
    "    </ul>\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"row\">\n" +
    "    <h5 class=\"text-center title-tab\">\n" +
    "      Properties\n" +
    "    </h5>\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"row aside-form\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "      <form class=\"form-horizontal\">\n" +
    "\n" +
    "        <div class=\"form-group element-fields\"\n" +
    "             data-ng-show=\"element.isVertex || element.isEdge\"\n" +
    "             data-ng-repeat=\"(key, value) in element.toJSON()\">\n" +
    "          <label class=\"col-md-6 control-label\" data-ng-bind=\"key\"></label>\n" +
    "          <div class=\"col-md-6\">\n" +
    "            <p class=\"form-control-static\" data-ng-bind=\"value\"></p>\n" +
    "          </div>\n" +
    "        </div>\n" +
    "\n" +
    "      </form>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/sections/home/api_doc.html',
    "<div>\n" +
    "  <div class=\"row\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "      <h2>[GET] Geo Social Ranking - /api/gsr</h2>\n" +
    "\n" +
    "      <p>\n" +
    "        This endpoint returns the top k ranked users users around a given query location q.\n" +
    "      </p>\n" +
    "\n" +
    "      <table class=\"table table-striped\">\n" +
    "        <thead>\n" +
    "          <tr>\n" +
    "            <th>Parameter</th>\n" +
    "            <th>Type</th>\n" +
    "            <th>Description</th>\n" +
    "          </tr>\n" +
    "        </thead>\n" +
    "\n" +
    "        <tbody>\n" +
    "          <tr>\n" +
    "            <td>lat</td>\n" +
    "            <td>required</td>\n" +
    "            <td>Float [latitude] point of query location.</td>\n" +
    "          </tr>\n" +
    "\n" +
    "          <tr>\n" +
    "            <td>lon</td>\n" +
    "            <td>required</td>\n" +
    "            <td>Float [longitude] point of query location.</td>\n" +
    "          </tr>\n" +
    "\n" +
    "          <tr>\n" +
    "            <td>K</td>\n" +
    "            <td>optional</td>\n" +
    "            <td>Integer [Top K-Ranks]. Defaults to 1.</td>\n" +
    "          </tr>\n" +
    "\n" +
    "          <tr>\n" +
    "            <td>radius</td>\n" +
    "            <td>optional</td>\n" +
    "            <td>Float [Number of kms]. Defaults to 4 kms.</td>\n" +
    "          </tr>\n" +
    "\n" +
    "          <tr>\n" +
    "            <td>data_set</td>\n" +
    "            <td>optional</td>\n" +
    "            <td>String [Dataset Name]. Defaults to 'data_GSR_Austin'.</td>\n" +
    "          </tr>\n" +
    "        </tbody>\n" +
    "      </table>\n" +
    "\n" +
    "      <h3>Sample API response</h3>\n" +
    "<pre>[\n" +
    "  {\n" +
    "    rank: 1,\n" +
    "    score: 0.35,\n" +
    "    userId: 2,\n" +
    "    distance: 0.25,\n" +
    "    friends: [\n" +
    "      2227, // USER-ID\n" +
    "      2.87824154026702 // DISTANCE\n" +
    "    ],[\n" +
    "      2042,\n" +
    "      2.11627756944029\n" +
    "    ],[\n" +
    "      354,\n" +
    "      1.88029914754779\n" +
    "    ],[\n" +
    "      1850,\n" +
    "      0.630553082673396\n" +
    "    ],\n" +
    "    nFriends: 3\n" +
    "  }\n" +
    "]\n" +
    "</pre>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/sections/home/concept_editor.html',
    "<div class=\"row\">\n" +
    "  <div class=\"col-md-12\">\n" +
    "    <div polar-concept-editor></div>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/sections/home/index.html',
    "<div>\n" +
    "\n" +
    "  <div class=\"row\">\n" +
    "    <div class=\"col-md-12\">\n" +
    "      <h2 class=\"text-center\">Geo Social Query Interface</h2>\n" +
    "\n" +
    "      <center>\n" +
    "        <img class=\"img-thumbnail img-responsive\" src=\"images/image03.png\" width=\"40%\" />\n" +
    "      </center>\n" +
    "\n" +
    "      <h4>Motivation</h4>\n" +
    "\n" +
    "      <p>Increasingly the past few years through GPS-enabled mobile devices, users publish their current geographical location to their friends, by “checking-in” at various places. Social networks, such as Facebook and Twitter, have been augmented with check-in functionality. This trend in geosocial networking has created opportunities for novel location-based social interactions and advertising. Given such a geosocial dataset we wish to build a browser based application which provides a map based query interface wherein advertisers/super-users can design geospatial queries and visualize the returned results.</p>\n" +
    "\n" +
    "      <hr />\n" +
    "\n" +
    "      <h4><a href=\"#/gsr\">Geo Social Ranking</a></h4>\n" +
    "\n" +
    "      <p>Given a query location q and k, rank the users of a Geo-Social Network based on their distance to q, the number of their friends in the vicinity of q, and the connectivity of those friends and returns the top-k users. The focus is exploring the variations in ranking functions to achieve various results suited for particular applications.</p>\n" +
    "\n" +
    "      <center>\n" +
    "        <img class=\"img-thumbnail img-responsive\" src=\"images/image00.png\" width=\"50%\" />\n" +
    "      </center>\n" +
    "\n" +
    "      <hr />\n" +
    "\n" +
    "      <h4>Nearest Friend Group</h4>\n" +
    "      Given a query location point q and integers k,m, find the k nearest groups of m users to q, such that the users in every group are connected through a common friend (star).\n" +
    "\n" +
    "      <blockquote>\n" +
    "        “the next group of five people who come to the restaurant will receive 20% discount”\n" +
    "      </blockquote>\n" +
    "\n" +
    "      <hr />\n" +
    "\n" +
    "      <h4>Geo-Social Keyword Search</h4>\n" +
    "\n" +
    "      <p>Top-k Nearest, Popular and Relevant users query that, given a query location q and a set of terms Tq, outputs the top-k users based on their proximity to q, their social connectivity, and the similarity of their profiles to Tq. Top-k Nearest Socially and Textually Relevant POIs query , which, given a user v and a set of terms Tq, returns the top-k POIs based on their proximity to v, the number of check-ins by friends of v, and their similarity to Tq. Top-k Frequent Social Keywords in Range query that discovers the top-k keywords based on their frequency in pairs of friends located within a geographic area.</p>\n" +
    "      <p>Each query covers a wide range of real-world tasks, including advertising, context-based search, and market analysis. Some expected visualizations and output of the queries are highlighted below.</p>\n" +
    "\n" +
    "      <center>\n" +
    "        <img class=\"img-thumbnail img-responsive\" src=\"images/image01.png\" width=\"40%\" />\n" +
    "        <img class=\"img-thumbnail img-responsive\" src=\"images/image02.png\" width=\"40%\" />\n" +
    "        <img class=\"img-thumbnail img-responsive\" src=\"images/image04.png\" width=\"40%\" />\n" +
    "      </center>\n" +
    "\n" +
    "    </div>\n" +
    "  </div>\n" +
    "\n" +
    "\n" +
    "\n" +
    "\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/sections/query/gsk.html',
    "<div>\n" +
    "  <div polar-gsr\n" +
    "    data-types=\"['GSK_FSKR']\"\n" +
    "    data-query-type=\"'gsk'\"></div>\n" +
    "</div>\n" +
    "\n"
  );


  $templateCache.put('app/scripts/sections/query/gsp.html',
    "<div>\n" +
    "  <div polar-gsr\n" +
    "    data-types=\"['GSK_NSTP']\"\n" +
    "    data-query-type=\"'gsp'\"></div>\n" +
    "</div>\n" +
    "\n"
  );


  $templateCache.put('app/scripts/sections/query/gsr.html',
    "<div>\n" +
    "  <div polar-gsr\n" +
    "    data-types=\"['GSK_NPRU','NSG_LAZY','LCA_UD','LCA_DD','RCA','HGS_UD','HGS_DD','GST_EAGER','GST_LAZY']\"\n" +
    "    data-query-type=\"'gsr'\"></div>\n" +
    "</div>\n" +
    "\n"
  );


  $templateCache.put('app/scripts/util/templates/alert.html',
    "<div>\n" +
    "  <div class=\"modal-body\" data-ng-if=\"data.head\">\n" +
    "    {{ data.head }}\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"modal-body\">\n" +
    "    {{ data.message }}\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"modal-footer\">\n" +
    "    <button class=\"btn btn-info\" type=\"button\" ng-click=\"ok()\">Ok</button>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/util/templates/dialog.html',
    "<div>\n" +
    "  <div class=\"modal-body\" data-ng-if=\"data.head\">\n" +
    "    {{ data.head }}\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"modal-body\">\n" +
    "    {{ data.message }}\n" +
    "  </div>\n" +
    "\n" +
    "  <div class=\"modal-footer\">\n" +
    "    <button class=\"btn btn-info\" type=\"button\" ng-click=\"ok()\">Ok</button>\n" +
    "    <button class=\"btn btn-danger\" type=\"button\" ng-click=\"cancel()\">Cancel</button>\n" +
    "  </div>\n" +
    "</div>\n"
  );


  $templateCache.put('app/scripts/util/templates/global_loader.html',
    "<div>\n" +
    "  <div>\n" +
    "    <div class=\"c-global-loader\" data-ng-if=\"loaderConfig\">\n" +
    "      <span data-ng-bind=\"loaderConfig.message\"></span>\n" +
    "    </div>\n" +
    "\n" +
    "    <div class=\"c-global-loader-center\" data-ng-if=\"loaderConfig && heavyLoading\">\n" +
    "      <center>\n" +
    "        <img src=\"images/loading.gif\" style=\"height: 120px;margin-right: 20px;\" />\n" +
    "        <br />\n" +
    "        Generating Insights\n" +
    "      </center>\n" +
    "    </div>\n" +
    "  </div>\n" +
    "</div>\n"
  );

}]);
