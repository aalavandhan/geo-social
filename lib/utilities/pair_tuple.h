class pair_tuple{

private:
    int id_1;
    int id_2;
    double score;

public:
    pair_tuple(int id_1, int id_2, double score): id_1(id_1), id_2(id_2), score(score){}

    int getId_1() const {return id_1;}
    int getId_2() const {return id_2;}
    double getScore() const {return score;}

    std::string get_as_string(){
        std::stringstream ss;
        ss << id_1 <<", "<<id_2<<", "<<score;
        return ss.str();
    }

};

struct tuple_score_descending_comp{

    bool operator()(pair_tuple  *x, pair_tuple  *y) const{
        return y->getScore() < x->getScore();
    }
};


struct tuple_id_ascending_comp{

    bool operator()(const pair_tuple  &x, const pair_tuple  &y) const{

        return (x.getId_1() < y.getId_1() || (x.getId_1() == y.getId_1() && x.getId_2() < y.getId_2()));
    }
};


struct tuple_score_descending_queue : public std::binary_function<pair_tuple*, pair_tuple*, bool>
{
    bool operator()(const pair_tuple* __x, const pair_tuple* __y) const {
            return __x->getScore() < __y->getScore();
    }
};

struct tuple_score_descending_queue_onstack : public std::binary_function<pair_tuple, pair_tuple, bool>
{
    bool operator()(const pair_tuple &__x, const pair_tuple &__y) const {
            return __x.getScore() < __y.getScore();
    }
};

