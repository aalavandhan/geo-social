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

public:

    topkGSK(IGPOs *spatialIndex, ISPOs *socialGraph, InvertedIndexGen *ivgen , SimpleQueries *simpleQueries);
    ~topkGSK();
	
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* naiveAlgorithm(double x, double y, unsigned int k, double w, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* SimpleTA(double x, double y, unsigned int k, int incrStep, double w, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* SimpleNRA(double x, double y, unsigned int k, int incrStep, double w, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* GridBasedTA(double x, double y, unsigned int k, double w, double _radius, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* GridBasedTA_bloom(double x, double y, unsigned int k, double w, double _radius, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* GridBasedTA_naive(double x, double y, unsigned int k, double w, double _radius, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* GridBasedNRA(double x, double y, unsigned int k, double w, double r, vector<string> *terms);
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* testBLOOM(double x, double y, int k, double w, double r, vector<string> *terms);

};