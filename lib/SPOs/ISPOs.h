#include <unordered_set>

class ISPOs
{
public:
    virtual void getFriends(int id, int*& friends,unsigned int& size) = 0;
	virtual unordered_set<int>* getFriends(int id) = 0;
    virtual bool areFriends(int user1, int user2) = 0;
	virtual int getUserDegree(int id ) =0;
	virtual multiset<my_pair, pair_comparator_descending>* getDegreeSet() = 0;
};
