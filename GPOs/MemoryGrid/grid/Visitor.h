class Visitor
{
private:

    double x;
    double y;
    double best_dist;
    vector<res_point*>* results;
    int visits;
    int computations;

public:
    Visitor();
    virtual ~Visitor();

    virtual void setResult(int id, double dist, double x, double y);
    virtual void setBestDist(double bestDist);
    virtual void setX(double x_c);
    virtual void setY(double y_c);

    virtual double getBestDist();
    virtual double getX();
    virtual double getY();
    virtual vector<res_point*>* getResult();


    // For statistics
    virtual void setStatistics(int visitedCells, int computedDistances);
    virtual int getVisits();
    virtual int getComputations();

};


