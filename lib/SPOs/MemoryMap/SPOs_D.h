class SPOs_D : public ISPOs
{
private:
    double totalCPUTime;
    double totalTime;
    //		map<int, Value*> hashTable;
    //		int valueCapacity;
    //		char hashTableFileName[100];
    int areFriendsExecutions, getFriendsExecutions, cacheHits;
    Utilities util;


    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
		
		
	map<int, pair<int*, int>* > cachedFriends;
	map<int, pair<int*, int>* >::iterator cacheIter;
    char buffer[256];



public:
    SPOs_D(int port, char* host);
    ~SPOs_D() {}

    // to be implemented by ISPOs
    virtual void getFriends(int id, int*& friends,unsigned int& numOfFriends);
    virtual bool areFriends(int user1, int user2);
	virtual multiset<my_pair, pair_comparator_descending>* getDegreeSet();
	multiset<my_pair, pair_comparator_descending>* degreeSet;
	virtual int getUserDegree(int id);

    int getAreFriendsExecutions();
    int getGetFriendsExecutions();
	int getCacheHits();

    double getTotalCPUTime();
    double getTotalTime();
	
	void clearExecutionHistory();

};
