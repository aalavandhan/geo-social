class IncrVisitor
{
private:

    double x;
    double y;
    double best_dist;
    list<Point*> pointersToResults;
    vector<res_point*>* results;
    priority_queue<Cell*, vector<Cell*>, Cell::ascending>* H;
    int ULevel, RLevel, DLevel, LLevel;
    int next;

public:

    bool finished;
    IncrVisitor();
    IncrVisitor(double _x, double _y);
    virtual ~IncrVisitor();

    virtual void setResult(int id, double dist, double x, double y);
    virtual void setBestDist(double bestDist);
    virtual void setLevels(int U, int R, int D, int L);
    virtual void setX(double x_c);
    virtual void setY(double y_c);

    virtual double getBestDist();
    virtual int getULevel();
    virtual int getRLevel();
    virtual int getDLevel();
    virtual int getLLevel();
    virtual double getX();
    virtual double getY();
    virtual priority_queue<Cell*, vector<Cell*>, Cell::ascending>* getHeap();
    virtual list<Point*>* getPointersToResults();
    virtual vector<res_point*>* getResult();

    virtual res_point* getNext();
};


