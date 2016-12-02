class my_pair{

private:
	int id;
	double score;

public:
	my_pair(int id, double score): id(id), score(score){}

	int getId() const {return id;}
	double getScore() const {return score;}

};

struct pair_comparator_descending{

	bool operator()(const my_pair  &x, const my_pair  &y){
		return y.getScore() < x.getScore();
	}
	
	bool operator()(const my_pair  *x, const my_pair  *y){
		return y->getScore() < x->getScore();
	}
	
};

struct MatchPairSecond
{
	MatchPairSecond(double &y) : x(y) {}

	bool operator()(const my_pair &val) {
		return (val.getScore() == x);
	}

	double &x;
};


struct MatchPairFirst
{
	MatchPairFirst(int &y) : x(y) {}

	bool operator()(const my_pair &user) {
		return (user.getId() == x);
	}

	int &x;
};