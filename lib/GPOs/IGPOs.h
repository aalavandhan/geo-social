class IGPOs
{
public:
    virtual void getLocation(int id, double* result) = 0;
    virtual vector<res_point*>* getkNN(double x, double y, int k) = 0;
    virtual vector<res_point*>* getRange(double x, double y, double radius) = 0;
    virtual set<res_point*, res_point_ascending_id>* getSetRange(double x, double y, double radius) = 0;
    virtual res_point* getNextNN(double x, double y, int incrStep) = 0;
	virtual res_point* getNextNN(double x, double y) = 0;
    virtual void clearNextNN() = 0;
    virtual vector<res_point*>* getRangeSortedId(double x, double y, double radius) = 0;
    virtual double estimateNearestDistance(double x, double y, int k) = 0;
};

