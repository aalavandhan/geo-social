class Utilities{

public:
    Utilities();
    ~Utilities();

    double print_time(struct timeval &start, struct timeval &end);
    double getX(int p, int mod);

    bool binarySearch(int sortedArray[], int first, int last, int key);
    bool binarySearch(vector<int>* sortedArray, int first, int last, int key);
    res_point* binarySearch_res_point(vector<res_point*>* sortedArray, int first, int last, int key);
    void sortResPoint_AscendingId(vector<res_point*>* vc);
    void sortResPoint_AscendingDist(vector<res_point*>* vc);

    double computeMinimumDistance(double x1, double y1, double x2, double y2);

    void updateUsersInValidRange(vector<res_point*>* res, double thres);

    //		Group* computeMyGroup(vector<res_point*>* usersInRange, int* friends, int friendsSize, res_point* p, int m);

    res_point* copy(res_point* toBeCopied);
    res_point* createResultPoint(int id, double x, double y, double distance);
    int getRandomInteger(int min, int max);

    void addToSortedList(int *list, int listSize, int newElement);
};
