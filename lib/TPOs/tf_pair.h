class tf_pair{

private:
    int id;
    int term_frequency;

public:
    tf_pair(int _id, int _term_frequency): id(_id), term_frequency(_term_frequency){}

    int getId() const {return id;}
    int getTF() const {return term_frequency;}
};

struct pair_comparator_ascending{

    bool operator()(const tf_pair  &_x, const tf_pair  &_y){
        return _x.getId() < _y.getId();
    }
};

struct pair_descending_frequency{

    bool operator()(const tf_pair  &_x, const tf_pair  &_y){
        return _y.getTF() < _x.getTF();
    }
};


struct MatchTFPairFirst{

	MatchTFPairFirst(int &y) : x(y) {}

	bool operator()(const tf_pair &user) {
		return (user.getId() == x);
	}

	int &x;
};