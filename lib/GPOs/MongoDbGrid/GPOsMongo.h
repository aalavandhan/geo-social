class GPOsMongo : public IGPOs
{

private:
	DBClientConnection c;
	string coll;
	vector<res_point*>* nextNNList;
	int computedNN, returnedNN, finalNextNN;
	Utilities util;

	bool flagNextNN;

	int kNNExecutions;
	int LocationExecutions;
	int NextNNExecutions;
	int RangeExecutions;

	double totalCPUTime;
	double totalTime;

	bool measurekNNTime;

public:
	GPOsMongo(string server, string collection);
	~GPOsMongo();

	virtual void getLocation(int id, double* result);
    virtual res_point* getNextNN(double x, double y, int incrStep);
    virtual vector<res_point*>* getkNN(double x, double y, int k);
    virtual vector<res_point*>* getRange(double x, double y, double radius);
    virtual set<res_point*, res_point_ascending_id>* getSetRange(double x, double y, double radius);
    virtual void clearNextNN();
    virtual vector<res_point*>* getRangeSortedId(double x, double y, double radius);
    virtual double estimateNearestDistance(double x, double y, int k);


	void updateCheckin(int id, double x, double y);

	int getkNNExecutions();
	int getLocationExecutions();
	int getNextNNExecutions();
	int getRangeExecutions();

	double getTotalCPUTime();
	double getTotalTime();

	void setMeasurekNNTime(bool set);

};
