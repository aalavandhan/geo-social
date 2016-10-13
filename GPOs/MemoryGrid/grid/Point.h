class Point {	

public:
    Point ();
    Point (double x, double y, int id);
    virtual ~Point ();

    virtual double getX();
    virtual double getY();
    virtual int getID();
    virtual double computeMinDist(double x, double y);
    virtual void printDetails();
    virtual double getMinDist();

    //Overload the < operator.
    bool operator< (const Point& p) const;
    //Overload the > operator.
    bool operator> (const Point& p) const;

    struct ascending : public std::binary_function<Point*, Point*, bool>
    {
        bool operator()(const Point* __x, const Point* __y) const { return __x->p_minDist > __y->p_minDist; }
    };

private:

    int p_id;
    double p_minDist;
    double p_x, p_y;

};


