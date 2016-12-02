struct res_point{
    int id;
    double x;
    double y;
    double dist;
};

struct res_point_equal : public binary_function<res_point*, res_point*, bool>
{
    bool operator()(const res_point* __x, const res_point* __y) const { return __x->id == __y->id; }
};

struct res_point_ascending_dist : public binary_function<res_point*, res_point*, bool>
{
    bool operator()(const res_point* __x, const res_point* __y) const { return __x->dist < __y->dist; }
};

struct res_point_descending_dist : public binary_function<res_point*, res_point*, bool>
{
    bool operator()(const res_point* __x, const res_point* __y) const { return __x->dist > __y->dist; }
};

struct res_point_ascending_id : public binary_function<res_point*, res_point*, bool>
{
    bool operator()(const res_point* __x, const res_point* __y) const { return __x->id < __y->id; }
};

struct res_point_descending_id : public binary_function<res_point*, res_point*, bool>
{
    bool operator()(const res_point* __x, const res_point* __y) const { return __x->id > __y->id; }
};

