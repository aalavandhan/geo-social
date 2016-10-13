class SimpleQueries{

private:
    IGPOs *gpos;
    ISPOs *spos;
    Utilities util;
    char f[3];
public:
    SimpleQueries(IGPOs *spatialIndex, ISPOs *socialGraph);
    ~SimpleQueries();

    Group* getClosestFriends3(double x, double y, res_point* user, unsigned int m);
    Group* getClosestFriends3(double x, double y, res_point* user, unsigned int m, char* function);
    Group* getClosestFriends2(double x, double y, res_point* user, unsigned int m, int incrStep);
    Group* getClosestFriends1(double x, double y, res_point* user, unsigned int m, int incrStep);

    Group* getMyFriendsInRange1(double x, double y, double radius, res_point* user);
    Group* getMyFriendsInRange2a(double x, double y, double radius, res_point* user);
    Group* getMyFriendsInRange2b(vector<res_point*>* seen_users, res_point* user);
    Group* getMyFriendsInRange3(double x, double y, double radius, res_point* user);

    Group* getMyFriendsInRange3(double x, double y, double radius, res_point* user, unsigned int m);
    //		Group* getMyFriendsInRange1(double x, double y, double radius, res_point* user, int m);


    Group* computeMyGroup(vector<res_point*>* usersInRange, int* friends, int friendsSize, res_point* p, unsigned int m);
    //returns the mGroup of the closest user to q who has an mGroup
    double getFirstmGroup(double x, double y, int m, int k);
};

