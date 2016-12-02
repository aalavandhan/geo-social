#include "Pair.h"

class User{

	public: User(int _size, int _id);
		Pair* friends;
		int id;
		int size;
		double totalWeight;
		~User();
		Pair* getFriends(); 
		int getFriendsSize();
		void setFriends(Pair* _friends, int _size);
		void printUser();
		void setTotalWeight(double weight);
		double getTotalWeight();
};
