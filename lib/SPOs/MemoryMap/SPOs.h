class SPOs : public ISPOs
{
private:
    double totalCPUTime;
    double totalTime;
    map<int, Value*> hashTable;
//    Value* hashTable2[DATASET_SIZE];
    int valueCapacity;
    char hashTableFileName[100];
    int areFriendsExecutions, getFriendsExecutions;
    Utilities util;

public:
    SPOs();
    ~SPOs() {}

    vector<int> ids;

    int load(const char* file);
	int load(const char* file, int count);
//    int loadfriends(const char* file);
//    void addFriendship(int user1, int user2);

    // to be implemented by ISPOs
    virtual void getFriends(int id, int*& friends,unsigned int& numOfFriends);
    virtual bool areFriends(int user1, int user2);
	virtual int getUserDegree(int id);
	void printTriangles( int id, vector<int> friendList);
	
    int getAreFriendsExecutions();
    int getGetFriendsExecutions();

    double getTotalCPUTime();
    double getTotalTime();

};
