

class Cell;

class QuadTreeNode
{

private:

	double *x1, *x2, *y1, *y2;


	//std::unordered_set<Point*> occupants; // if needed	

	// const unsigned int level;

	//if used as a cell
	// Cell type, it is used for the heaps
	// #define CELL 0
	// QuadTreeNode types used for heap
	// #define QTN 1
	// #define USER 2

	int node_type;   // 
	int user_id;


public:	

	struct descending_score : public std::binary_function<QuadTreeNode*, QuadTreeNode*, bool>
    {
        bool operator()(const QuadTreeNode* __x, const QuadTreeNode* __y) const {

            if(__x->bounding_score < __y->bounding_score){
                return true;
            }
            else if(__x->bounding_score == __y->bounding_score) {
                if(__x->c_minDist > __y->c_minDist) {
                    return true;
                }
                else  if(__x->c_minDist == __y->c_minDist){
                    if(__x->max_degree > __y->max_degree)
                        return true;
                }
            }
            return false;

        }

    };
	
	struct descending_score_for_multiSet : public std::binary_function<QuadTreeNode*, QuadTreeNode*, bool>
    {
        bool operator()(const QuadTreeNode* __x, const QuadTreeNode* __y) const {

            if(__x->bounding_score > __y->bounding_score){
                return true;
            }
            else if(__x->bounding_score == __y->bounding_score) {
                if(__x->c_minDist < __y->c_minDist) {
                    return true;
                }
                else  if(__x->c_minDist == __y->c_minDist){
                    if(__x->max_degree < __y->max_degree)
                        return true;
                }
            }
            return false;

        }

    };
	
	
	// boost::heap::fibonacci_heap<QuadTreeNode*, boost::heap::compare<QuadTreeNode::descending_score>>::handle_type handle;
	// boost::heap::fibonacci_heap<QuadTreeNode*, boost::heap::compare<QuadTreeNode::descending_score>> testing_heap;
	unsigned int numOccupants;
	unsigned int numPointsOfInterest;

	QuadTreeNode* pParentNode;
	unsigned int level;
	int length;
	bool hasChildren;
	vector<QuadTreeNode*>* children;
	vector<Cell*>* cellMembers;

	/*
	-----------------
	|	2	|	4	|
	|	01	|	11	|
	--------|-------|
	|	00	|	10	|
	|	1	|	3	|
	-----------------
	*/
	QuadTreeNode(double _x1, double _y1, double _x2, double _y2, unsigned int _level, unsigned int _length, QuadTreeNode* pParent = NULL);
	
	QuadTreeNode(QuadTreeNode* _qtn);
	
	~QuadTreeNode();
	
	double getX1();
    double getY1();
    double getX2();
    double getY2();
	
	void setType(int type);
	int getType();
	
	int getID();
	void setID(int id);

	//temporary query variables - clear after query processing
	double computeMinDist(double x, double y);
	double computeMaxDist(double x, double y);
	void setMinDist(double dist);
	double getMinDist();
	double computeScore();
	
	double computeMulScore();
	void setTFIDF(double _value);
	double computePOIScore();
	void setBloomVNP(double _vnp_score);
	void setPerccentileScore(double _score);

	//FOR USERS
	//max impact idx
	unordered_map<string, double >* idxImpact;
	
	//social max
	int max_degree;
	
	//geo max
	double c_minDist;
	//geo max
	double c_maxDist;
	
	//text max
	double max_tfidf;
	
	//FOR POIs
	//social POI
	double vnp_score;
	
	unordered_map<string, double >* P_idxImpact;

    //Instantiate Bloom Filter
    bloom_filter* filter;
	
	//scaling bloom filter and count
	scaling_bloom_t* bloom;
	int history_checkin_count;
	
	double bounding_score;
	//QuadTreeNode
	
	double diversity_score;
	double real_score;
	

	friend class Grid;
};
