class Value {
public:
    int id;
    int* list;
    int size;

public:
    Value(int size, int id);
    ~Value();
    int* getList();
    int getListSize();
    void setList(int* _list, int _size);
};
