#include <unordered_set>
#include <unordered_map>

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
	string int_to_string(int);

    void addToSortedList(int *list, int listSize, int newElement);
	double getTextDistanceBetween(unordered_set<string>* user_profile_1, unordered_set<string>* user_profile_2);
	double getTextDistanceBetween( unordered_map<string, double>* user_profile_1, unordered_set<string>* user_profile_2 );
	double getSocialDistanceBetween(unordered_set<int>* _f1, unordered_set<int>* _f2);
	double getDistanceBetween(int arr1[], int arr2[], int m, int n, string f);
	int countIntersection(int arr1[], int arr2[], int m, int n);
	int countUnion(int arr1[], int arr2[], int m, int n);
	double computeJaccard(int arr1[], int arr2[], int m, int n);
	double computeSetIntersection( unordered_set<int>* _f1, unordered_set<int>* _f2 );
};
