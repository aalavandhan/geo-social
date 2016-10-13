class Grid {

private:
    Cell* table[X+1][Y+1];
    int x, y;
    map<int , Point*>* locations;

public:
    Grid ();
    ~Grid ();

    Cell* getCell(double x, double y);
    Cell* getCell(int i, int j);
    bool addCheckIn(Point* user);
    void updateCheckIn(Point* p, double old_x, double old_y);
    Point* getPoint(int id);
    void getLocation(int id, Visitor& v);


    void getNextNN(IncrVisitor& v, int k);
    void getkNN(Visitor& v, int k);
    vector<res_point*>* getkNN(double x, double y, int k);
    void getRange(double x, double y, double radius, Visitor& v);
    vector<res_point*>* getRange(double x, double y, double radius);
    set<res_point*, res_point_ascending_id>* getSetRange(double x, double y, double radius);

    void getRange2(double x, double y, double radius, Visitor& v);

    void getRectangle(int direction, int level, double x, double y, Cell& c);

    bool loadFromFile(const char* fileName);
	bool loadFromFile(const char* fileName, int numOfFiles);

    list<Cell*>* getIntersectedCellsWithRectangle(double x1, double y1, double x2, double y2);
    double estimateNearestDistance(double x, double y, int k);

};

