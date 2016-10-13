#include "../../../headersMemory.h"

Visitor::Visitor(){

    x = 0.0;
    y = 0.0;
    best_dist = 99999.9;
    results = new vector<res_point*>();
}

Visitor::~Visitor(){

    while(!results->empty()) {
        delete results->back();
        results->pop_back();
    }
    delete results;

}

void Visitor::setResult(int id, double dist, double x, double y){

    res_point* rp = new res_point();

    rp->id = id;
    rp->x = x;
    rp->y = y;
    rp->dist = dist;

    results->push_back(rp);

    //pointersToResults.push_back(&p);

}

vector<res_point*>* Visitor::getResult(){
    return results;
}

void Visitor::setBestDist(double bestDist){
    best_dist = bestDist;
}

double Visitor::getBestDist(){
    return best_dist;
}


void Visitor::setX(double x_c){
    x = x_c;
}

void Visitor::setY(double y_c){
    y = y_c;
}

double Visitor::getX(){
    return x;
}

double Visitor::getY(){
    return y;
}


void Visitor::setStatistics(int visitedCells, int computedDistances){

    visits = visitedCells;
    computations = computedDistances;
}


int Visitor::getVisits(){

    return visits;

}


int Visitor::getComputations(){

    return computations;
}

/*
    list<Point*>* Visitor::getPointersToResults(){
        return &pointersToResults;
    }
*/
