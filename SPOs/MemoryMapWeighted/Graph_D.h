class Graph_D : public ISPOs
{
private:
    double totalCPUTime;
    double totalTime;
    //		map<int, Value*> hashTable;
    //		int valueCapacity;
    //		char hashTableFileName[100];
    int areFriendsExecutions, getFriendsExecutions;
    Utilities util;


    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];



public:
    Graph_D(int port, char* host);
    ~Graph_D() {}

    // to be implemented by ISPOs
    virtual void getFriends(int id, int*& friends,unsigned int& numOfFriends);
    virtual bool areFriends(int user1, int user2);
	virtual int getUserDegree(int id);

    int getAreFriendsExecutions();
    int getGetFriendsExecutions();

    double getTotalCPUTime();
    double getTotalTime();

};
