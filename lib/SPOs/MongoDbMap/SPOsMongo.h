class SPOsMongo : public ISPOs
{
      
	private:     

		double totalTime;
		double totalCPUTime;
		DBClientConnection c;
		string coll;

		int getFriendsExecutions;
		int areFriendsExecutions;

		Utilities util;
      
     	public: 
		SPOsMongo(string server, string collection);
		~SPOsMongo();
             
		virtual void getFriends(int id, int*& friends, unsigned int& size);
		virtual bool areFriends(int user1, int user2);
		virtual int getUserDegree(int id );
		void printTriangles( int id, vector<int> friendList);

		int getAreFriendsExecutions();
		int getGetFriendsExecutions();

		double getTotalCPUTime();
        double getTotalTime();

		void addFriendship(int user1, int user2);
		void setFriends(int id, int*& friends, unsigned int& numOfFriends);
};
