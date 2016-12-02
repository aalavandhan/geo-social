class GPOs : public IGPOs
{

private:
    unordered_map<int, Point*> locations;
    int kNNExecutions;
    int LocationExecutions;
    int NextNNExecutions;
    int RangeExecutions;
    double totalCPUTime, totalTime;
	
	map<int, double*> cachedLocs;

    // nextNN without the incremental approach of NN
    vector<res_point*>* nextNNList;
    int computedNN, returnedNN, finalNextNN;
    bool flagNextNN;
    int objects;
    Utilities util;
	IncrVisitor *incrVis;
	int pureNNexec;
	SPOs* spos;
	int LOADING_FACTOR = DATASET_SIZE;

public:
	unordered_map<int, Point*> poi_locations;
	unordered_map<int, unordered_set<int>* > POI_historyMap;
    Grid *grid;
    GPOs (char* gridFileName, const string& file, SPOs* _spos);
    GPOs();
    ~GPOs ();
    vector<int> ids;
	vector <QuadTreeNode*>* rootNode;
	
	map<string, int> IDF;
	map<string, int> P_IDF;
	unordered_map<int, vector<my_pair>* >* ST_Matrix;  
	unordered_map<int, unordered_map<int,double>* >* ST_Score_Map;
	
	unordered_map<int, unordered_set<string>* >* user_profile_map = new unordered_map<int, unordered_set<string>* >();
	unordered_map<int, unordered_set<string>* >* poi_profile_map = new unordered_map<int, unordered_set<string>* >();
	
	//summary information of P_IDF
	//keep mean, sd, probabilty
	
	virtual res_point* getNextNearestUser(double x, double y, int signal);
	
	virtual res_point* getNextNearestNeighbourWithTerm(double x, double y, int signal, vector<string>* terms);

	virtual vector<res_point*>* getkNNfromIDX(double x, double y, int k, vector<string>* terms);
    virtual void getLocation(int id, double* result);
    virtual res_point* getNextNN(double x, double y, int incrStep);
	virtual res_point* getNextNN(double x, double y);
    virtual vector<res_point*>* getkNN(double x, double y, int k);
    virtual vector<res_point*>* getRange(double x, double y, double radius);
    virtual set<res_point*, res_point_ascending_id>* getSetRange(double x, double y, double radius);
    virtual vector<res_point*>* getRangeSortedId(double x, double y, double radius);
    virtual double estimateNearestDistance(double x, double y, int k);
    virtual void clearNextNN();
	
	virtual counting_bloom_t* getRangeBloom(double x, double y, double radius);
	virtual Cell* getCell(double x, double y);
	virtual void loadPOIeverything(const char* locationFile,const char* historyFile,const char* keywordfile,const char* frequencyFile);
	virtual unordered_map<int, Point*>* getAllPOIlocations();
	//text fucntions
	bool loadTextFile(const std::string&);
	bool loadIDF(const char* fileName);
	virtual int getP_IDF(string word);
	virtual int getIDF(string word);
	
	virtual unordered_set<string>* getUserProfile( int _id);
	virtual unordered_set<string>* getPOIProfile( int _id);
	
	virtual unordered_set<int>* getHistoryCheckins(int poi_id);
	
	virtual set<res_point*, res_point_ascending_dist>* getRangeWithIDX(double x, double y, double radius, vector<string> *terms, vector<vector<multiset<tf_pair, pair_descending_frequency>*>*> *_keywordVEC);
	virtual void* getIDXinRange(double x, double y, double radius, void* posting_list_map);
	virtual void* getIDXinRange_ordered(double x, double y, double radius, void* posting_list_map);
	void sendSPOStoGRID(SPOs* _spos);
	vector <QuadTreeNode*>* getQuadTree();
	void loadHistoryCheckins(const char * historyFile);
	virtual void createQuadTree();
	virtual bool isUserInRange(double x, double y, double radius, int f_id);

    // nextNN without the incremental approach of NN
    //virtual res_point* getNextNN(double x, double y, int incrStep);

    int getPageAccesses();
    void resetPageAccesses();

    int getkNNExecutions();
    int getLocationExecutions();
    int getNextNNExecutions();
    int getRangeExecutions();

    bool loadLocations(const char* fileName);


    double getTotalCPUTime();
    double getTotalTime();

    void updateCheckin(Point* p);
	
};
