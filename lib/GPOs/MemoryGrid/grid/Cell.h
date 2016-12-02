class Cell {

public:
    struct ascending : public binary_function<Cell*, Cell*, bool>
    {
        bool operator()(const Cell* __x, const Cell* __y) const { return __x->c_minDist > __y->c_minDist; }
    };
    
public:
    Cell ();
    virtual ~Cell();

    virtual void newCheckIn(Point* user, int degree);
    virtual double computeMinDist(double x, double y);
    virtual double computeMaxDist(double x, double y);
    virtual bool intersectsWithCircle(double x, double y, double radius);
    virtual bool fullyCoveredByCircle(double x, double y, double radius);
    virtual double intersectedVolumeWithRectangle(double x_1, double y_1, double x_2, double y_2);

    virtual void setDimensions(double x_1, double y_1, double x_2, double y_2);
    virtual void setID(int id);
    virtual void setType(int type);
    virtual void setDirection(int direction);
    virtual void setMinDist(double dist);
    void deleteCheckIn(Point* p);


    virtual list<Point*>* getCheckIns();
	
    virtual double getX1();
    virtual double getY1();
    virtual double getX2();
    virtual double getY2();
    virtual int getID();
    virtual int getType();
    virtual int getDirection();
    virtual double getMinDist();
    void setIndex(int c_i, int c_j);
    int getIndexI();
    int getIndexJ();
	
	virtual scaling_bloom_t* getBloomFilter();
	bool isUserInCell(int f_id);

	
	//text
	void newSIDX(string word,int occurrences,int user_id);
	
	
	multiset<tf_pair, pair_descending_frequency>* lookupWord(const string& word);
	
	//for users
	map<string, multiset<tf_pair, pair_descending_frequency>* > idxOfSet;
	unordered_map<int,double> docLenMap;
	double getDocLen(int _user_id);
	unordered_map <string,multiset<my_pair, pair_comparator_descending>* >* idxOfImpacts;
	multiset<my_pair, pair_comparator_descending>* lookupWordInImpacts(const string& word);
	multiset<my_pair, pair_comparator_descending>* lookupWordIn_P_Impacts(const string& word);
	
	virtual list<Point*>* getPointsOfInterest();
	void newPOI(Point* poi);
	void newPOIHistoryEntry(int poi_id, int history_checkin);
	void newPOIidxEntry(string word, int occurrences, int poi_id);
	
	//max impact values only
	unordered_map<string, double >* idxImpact;
	
	//FOR POIs
	//poi id - > unordered set of past checkins
	unordered_map< int, unordered_set<int>*> P_historyMap;
	
	
	map<string, multiset<tf_pair, pair_descending_frequency>* > P_idxOfSet;
	//document lenght of POIs
	unordered_map<int,double> P_docLenMap;
	
	//keeps inverted file
	unordered_map <string,multiset<my_pair, pair_comparator_descending>* >* P_idxOfImpacts;
	//keeps max impacts
	unordered_map<string, double >* P_idxImpact;
	
	int max_degree;
	bloom_filter* filter;
	QuadTreeNode* pParentNode; // 
	scaling_bloom_t* bloom;

private:
	bool hasFilter;
    double *x1, *x2, *y1, *y2;
    list<Point*>* checkIns;
	list<Point*>* points_of_interest;
    int cell_type;
    int c_id;
    int c_direction;
    double c_minDist;
    double c_maxDist;
    int numOfPoints;
	int history_checkin_count;
    int i, j;
	
};

