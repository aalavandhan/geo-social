class QuadTreeNode;
struct counting_bloom_t;
class Cell;
class pair_tuple;
#include <unordered_map>
#include <unordered_set>

class IGPOs
{
public:
    virtual void getLocation(int id, double* result) = 0;
    virtual vector<res_point*>* getkNN(double x, double y, int k) = 0;
    virtual vector<res_point*>* getRange(double x, double y, double radius) = 0;
    virtual set<res_point*, res_point_ascending_id>* getSetRange(double x, double y, double radius) = 0;
    virtual res_point* getNextNN(double x, double y, int incrStep) = 0;
    virtual void clearNextNN() = 0;
    virtual vector<res_point*>* getRangeSortedId(double x, double y, double radius) = 0;
    virtual double estimateNearestDistance(double x, double y, int k) = 0;
	virtual int getP_IDF(string word) = 0;
	virtual int getIDF(string word) = 0;
	virtual unordered_set<string>* getUserProfile( int _id) = 0;
	virtual unordered_set<string>* getPOIProfile( int _id)= 0;
	virtual unordered_set<int>* getHistoryCheckins(int poi_id)= 0;
	virtual unordered_map<int, Point*>* getAllPOIlocations() = 0;
	
	virtual counting_bloom_t * getRangeBloom(double x, double y, double radius) = 0;
	virtual bool isUserInRange(double x, double y, double radius, int f_id) = 0;
	virtual set<res_point*, res_point_ascending_dist>* getRangeWithIDX(double x, double y, double radius, vector<string> *terms, vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* _keywordVEC) = 0;
	virtual void* getIDXinRange(double x, double y, double radius, void* posting_list_map)=0;
	virtual void* getIDXinRange_ordered(double x, double y, double radius, void* posting_list_map)=0;
	virtual vector <QuadTreeNode*>* getQuadTree() = 0;
	virtual Cell* getCell(double x, double y) = 0;
	virtual res_point* getNextNearestUser(double x, double y, int signal) = 0;
	virtual vector<res_point*>* getkNNfromIDX(double x, double y, int k, vector<string>* terms) = 0;
	virtual res_point* getNextNearestNeighbourWithTerm(double x, double y, int signal, vector<string>* terms) = 0;
	virtual void loadPOIeverything(const char* locationFile,const char* historyFile,const char* keywordfile,const char* frequencyFile) = 0;
	virtual void createQuadTree() = 0;
};

