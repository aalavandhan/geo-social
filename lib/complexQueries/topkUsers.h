class topkUsers{

private:
    IGPOs *gpos;
    ISPOs *spos;
    SimpleQueries *sq;
    Utilities util;
    int usersChecked;
    double threshold;
    double current_dist;
    int m_group;
	map<int, list<pair<int, int> >* > triangles;
	// map<int, list<pair<int, int> >* > completeTriangles;
	vector<res_point*>* seen_usersVEC;
	map<int, int> counterMap;
	
	map < int, list< multiset<double, greater<double> >::iterator >* > map_of_iter ;
    multiset<double, greater<double> > set_of_scores;

public:

    topkUsers(IGPOs *spatialIndex, ISPOs *socialGraph, SimpleQueries *simpleQueries);
    ~topkUsers();
    double computeLBTime;

    int getNumOfUsersChecked();
    double getThreshold();
    priority_queue <Group*, vector<Group*>, Group::ascending_score >* LCA(double x, double y, int k, double w, double r);
    priority_queue <Group*, vector<Group*>, Group::ascending_score >* HGS(double x, double y, int k, double d);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* HGS2(double x, double y, int k, double d);
    Group* RC_BoundComputation(res_point* user, vector<res_point*>* seen_users, double w);
	Group* RC_ScoreComputation(double x, double y, res_point* user, vector<res_point*>* friendVEC, double w);
    priority_queue <Group*, vector<Group*>, Group::ascending_score >* RC(double x, double y,unsigned int k, int incrStep, double w);
    priority_queue <Group*, vector<Group*>, Group::ascending_score >* GCA(double x, double y,unsigned int k, int incrStep, double beta);
	double ScoreModuleGCA(res_point* user, map<int, double>* seen_usersMAP, vector <pair<int, int > >* seen_usersSOC, double beta);
	

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* GST_Lazy(double x, double y,unsigned int k, int incrStep, double beta);
	double GST_Lazy_Bound(res_point* user, vector<double>* M_bestVEC, double beta);
	
	
	double print_time(struct timeval &start, struct timeval &end);
    double getDensity(Group* gp);
	double getOverlapInTopK(vector<int>* resultSet);
	double getOverlapInFriends(vector<int>* resultSet);
	double getInterDivUnion(vector<int>* resultSet);
};

