class topkGSK{

private:
    IGPOs *gpos;
    ISPOs *spos;
	InvertedIndexGen *ivgen;
    SimpleQueries *sq;
    Utilities util;
    int usersChecked;
    double threshold;
	double print_time(struct timeval &start, struct timeval &end);
	
	//class variables for NTK
	int m_group;
	map<int, list<pair<int, int> >* > triangles;
	// map<int, list<pair<int, int> >* > completeTriangles;
	vector<res_point*>* seen_usersVEC;
	map<int, int> counterMap;
	
	map < int, list< multiset<double, greater<double> >::iterator >* > map_of_iter ;
    multiset<double, greater<double> > set_of_scores;

public:

    topkGSK(IGPOs *spatialIndex, ISPOs *socialGraph, InvertedIndexGen *ivgen , SimpleQueries *simpleQueries);
    ~topkGSK();
	int getNumOfUsersChecked();

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* NPRU(double x, double y, unsigned int k, double w, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* NSTP(double x, double y, unsigned int k, double w, int USER_, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* FSKR(double x, double y, unsigned int k, double w, double _radius);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* NSTP_VIS(double x, double y,  unsigned int k, double w, int USER_, vector<string> *terms);
};