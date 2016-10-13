class Graph : public ISPOs
{
	public: int V;
		Graph();
		Graph(int numOfUsers);
		User* graph;
		double load(const char* file);    //returns the total weight of the social edges
		void printGraph();
		User* getUser(int user);

		virtual void getFriends(int id, int*& friends,unsigned int& numOfFriends);
    	virtual bool areFriends(int user1, int user2);
		virtual int getUserDegree(int id);

};
