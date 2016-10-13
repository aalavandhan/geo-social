class Cell {

public:
    struct ascending : public binary_function<Cell*, Cell*, bool>
    {
        bool operator()(const Cell* __x, const Cell* __y) const { return __x->c_minDist > __y->c_minDist; }
    };
    
public:
    Cell ();
    virtual ~Cell();

    virtual void newCheckIn(Point* user);
    virtual double computeMinDist(double x, double y);
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


private:
    double *x1, *x2, *y1, *y2;
    list<Point*>* checkIns;
    int cell_type;
    int c_id;
    int c_direction;
    double c_minDist;
    int numOfPoints;
    int i, j;

};

