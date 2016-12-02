class Group {	      

public:
    int id;
    double lb;
    double user_dist;
    double adist;
    double longest_dist;
    int longest_dist_i;
    bool finished_flag;
    Utilities util;
    unsigned int m;
    double max_dist;
    double score;
	
	double userX;
	double userY;
	
    // Approach 1. You keep a vector of res_points, use mergesort O(nlogn), and you can copy it easily;
    // Total cost: create: O(nlogn), copy: O(n) => sum = O(nlogn)
    // Approach 2. you keep a priority_queue. the cost of insertion is O(logn), when you need to copy it, you
    // must copy it to two priority queus and then to reinsert it back (supports only push and pop).
    // Total cost: create: O(nlogn), copy: O(3nlogn)=> sum = O(nlogn). So, it is the same.

    priority_queue<res_point*, vector<res_point*>, res_point_ascending_dist>* friends = NULL;
	

public:
    Group();
    Group(Group* g);
    Group(res_point* user, int cardinality, char* function);
    Group(res_point* user);
	Group(Point* user);
    ~Group();

    double getAdist();
    void removeDuplicates();
	vector<int> giveFriends();
    void addFriend(res_point* _friend);
    priority_queue<res_point*, vector<res_point*>, res_point_ascending_dist>* getFriends();
    int size();
    void print();

	int getId();

    bool finished();
    void updateLongestDist(double distance);

    double getLongestDistance();


    struct ascending : public std::binary_function<Group*, Group*, bool>
    {
        bool operator()(const Group* __x, const Group* __y) const {
                return __x->adist > __y->adist;
        }
    };

    struct descending : public std::binary_function<Group*, Group*, bool>
    {
        bool operator()(const Group* __x, const Group* __y) const {
                return __x->adist < __y->adist;
        }

    };

    bool operator<( const Group* other) const
    {
        return id < other->id;
    }

    struct ascending_score : public std::binary_function<Group*, Group*, bool>
    {
        bool operator()(const Group* __x, const Group* __y) const {

            if(__x->score > __y->score){
                return true;
            }
            else if(__x->score == __y->score) {
                if(__x->adist < __y->adist) {
                    return true;
                }
                else  if(__x->adist == __y->adist){
                    if(__x->user_dist < __y->user_dist)
                        return true;
                }
            }
            return false;
        }
    };

    struct descending_score : public std::binary_function<Group*, Group*, bool>
    {
        bool operator()(const Group* __x, const Group* __y) const {

            if(__x->score < __y->score){
                return true;
            }
            else if(__x->score == __y->score) {
                if(__x->adist > __y->adist) {
                    return true;
                }
                else  if(__x->adist == __y->adist){
                    if(__x->user_dist > __y->user_dist)
                        return true;
                }
            }
            return false;

        }

    };


    struct ascendingLongDist : public std::binary_function<Group*, Group*, bool>
    {
        bool operator()(const Group* __x, const Group* __y) const { return __x->longest_dist > __y->longest_dist; }
    };

    struct descendingLongDist : public std::binary_function<Group*, Group*, bool>
    {
        bool operator()(const Group* __x, const Group* __y) const { return __x->longest_dist < __y->longest_dist; }
    };

};

