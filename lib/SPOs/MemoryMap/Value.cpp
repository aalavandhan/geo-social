#include "../../headersMemory.h"

Value::Value(int _size, int _id) {
    id = _id;
    size = _size;
    list = (int*)malloc(sizeof(int)*size);
}

Value::~Value(){
    delete list;
}

int* Value::getList(){             	
    int *t = (int*)malloc(sizeof(int)*size);

    for (int i = 0; i < size; i++){
        *(t+i) = list[i];
    }

    return t;
}

int Value::getListSize() {
    return size;
}

void Value::setList(int* _list, int _size){
    size = _size;

    for (int i = 0; i < _size; i++){
        *(list+i) = _list[i];
    }

    free(_list);
}

