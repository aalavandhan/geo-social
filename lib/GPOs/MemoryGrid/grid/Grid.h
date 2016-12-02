class Grid {

private:
    Cell* table[X+1][Y+1];

    int x, y;
    unordered_map<int , Point*>* locations;
	unordered_map<int , Point*>* poi_locations;
	Cell * lastCell;
	int lastUserId;
	int lastPOIId;
	double boundary_control;
	
	//class variables for incremental nextNearestUser (nnu)
    priority_queue<Cell*, vector<Cell*>, Cell::ascending>* nnu_heap;
	int nnu_levels[4];
	int nnu_count;
	
	//class variables for incremental nextNearestUserWithTerm (nnut)
	priority_queue<Cell*, vector<Cell*>, Cell::ascending>* nnut_heap;
	int nnut_levels[4];
	int nnut_count;
	

public:
    Grid ();
    ~Grid ();
	SPOs* spos;
	void setSPOs(SPOs* _spos);
	
	vector<QuadTreeNode*>* rootNode;
	
	res_point* getNextNearestUser(double x, double y, int signal);
	res_point* getNextNearestNeighbourWithTerm(double x, double y, int signal, vector<string>* terms);
	
	
	//PERCENTILE FUNCTIONS
	map<int, double>* createCumulativeHistogram(double x, double y, int numOfBins);
	
	
	//text
	bool addToSIDX(const string& word, int occurrences, Point* user);
	
	bool isUserInRange(double x, double y, double radius, int f_id);

    Cell* getCell(double x, double y);
    Cell* getCell(int i, int j);
    bool addCheckIn(Point* user);

    void updateCheckIn(Point* p, double old_x, double old_y);
    Point* getPoint(int id);
    void getLocation(int id, Visitor& v);
	
	//quad tree methods
	QuadTreeNode* buildNode(double x1, double y1, double x2, double y2, unsigned int level, unsigned int length, QuadTreeNode* parentNode);
	vector <QuadTreeNode*>* createQuadTree();
	void deleteQuadTree();
	vector<Cell*>* getCells(double x1, double y1, double x2, double y2);
	
	vector<res_point*>* getkNNfromIDX(double x, double y, int k, vector<string>* terms);
	
    void getNextNN(IncrVisitor& v, int k);
    void getkNN(Visitor& v, int k);
    vector<res_point*>* getkNN(double x, double y, int k);
    void getRange(double x, double y, double radius, Visitor& v);
    vector<res_point*>* getRange(double x, double y, double radius);
    set<res_point*, res_point_ascending_id>* getSetRange(double x, double y, double radius);
	set<res_point*, res_point_ascending_dist>* getRangeWithIDX(double x, double y, double radius, vector<string> *terms, vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* _keywordVEC);
	void* getIDXinRange(double x, double y, double radius, void* posting_list_map);
	void* getIDXinRange_ordered(double x, double y, double radius, void* posting_list_map);
    void getRange2(double x, double y, double radius, Visitor& v);
	
	bool setHistoryPOI(int poi_id, unordered_set<int>* history_set);
	bool addPOI(Point* poi);
	bool addPOIToSIDX(const string& word, int occurrences, Point* poi);
	void newPOIHistoryEntry(int poi_id, int history_checkin);

    void getRectangle(int direction, int level, double x, double y, Cell& c);

    bool loadFromFile(const char* fileName);
	

    list<Cell*>* getIntersectedCellsWithRectangle(double x1, double y1, double x2, double y2);
    double estimateNearestDistance(double x, double y, int k);
	counting_bloom_t* getRangeBloom(double x, double y, double radius);

};

