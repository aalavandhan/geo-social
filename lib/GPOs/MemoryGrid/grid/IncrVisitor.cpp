#include "../../../headersMemory.h"


IncrVisitor::IncrVisitor(double _x, double _y){

    finished = false;
    x = _x;
    y = _y;
    best_dist = 99999.9;
    next = 0;
    ULevel = RLevel = DLevel = LLevel = 0;
    results = new vector<res_point*>();
    H = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>();

}


IncrVisitor::IncrVisitor(){
    finished = false;
    x = 0.0;
    y = 0.0;
    best_dist = 99999.9;
    next = 0;
    ULevel = RLevel = DLevel = LLevel = 0;
    results = new vector<res_point*>();
    H = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>();

}

IncrVisitor::~IncrVisitor(){

    while(!results->empty()) {
        delete results->back();
        results->pop_back();
    }
    delete results;

    while(!H->empty()) {
        Cell* c = H->top();
        if(c->getType() != CELL)
            delete c;
        H->pop();
    }
    delete H;

}

void IncrVisitor::setResult(int id, double dist, double x, double y){

    res_point* rp = new res_point;

    rp->id = id;
    rp->x = x;
    rp->y = y;
    rp->dist = dist;
    results->push_back(rp);

    //pointersToResults.push_back(&p);

}

vector<res_point*>* IncrVisitor::getResult(){
    return results;
}

void IncrVisitor::setBestDist(double bestDist){
    best_dist = bestDist;
}

double IncrVisitor::getBestDist(){
    return best_dist;
}

void IncrVisitor::setLevels(int U, int R, int D, int L){
    ULevel = U;
    RLevel = R;
    DLevel = D;
    LLevel = L;
}

int IncrVisitor::getULevel(){
    return ULevel;
}

int IncrVisitor::getRLevel(){
    return RLevel;
}

int IncrVisitor::getDLevel(){
    return DLevel;
}

int IncrVisitor::getLLevel(){
    return LLevel;
}

void IncrVisitor::setX(double x_c){
    x = x_c;
}

void IncrVisitor::setY(double y_c){
    y = y_c;
}

double IncrVisitor::getX(){
    return x;
}

double IncrVisitor::getY(){
    return y;
}


priority_queue<Cell*, vector<Cell*>, Cell::ascending>* IncrVisitor::getHeap(){
    return H;
} 

list<Point*>* IncrVisitor::getPointersToResults(){
    return &pointersToResults;
}

res_point* IncrVisitor::getNext(){
    if(next>=DATASET_SIZE)
        return NULL;
    res_point* tmp = (*results)[next++];
    return tmp;
}

