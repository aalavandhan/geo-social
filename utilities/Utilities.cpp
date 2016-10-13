#include "../headers.h"

Utilities::Utilities(){

    srand((unsigned)time(NULL));

}

Utilities::~Utilities(){}


//time in microseconds
double Utilities::print_time(struct timeval &start, struct timeval &end){
    double usec;

    usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    return usec;
}


int Utilities::getRandomInteger(int min, int max){
    //srand((unsigned)time(NULL));
    return (rand()%(max-min+1))+min;

}

double Utilities::getX(int p, int mod){

    int x = rand()%mod;
    int x2 = rand()%1000;

    double f2 = (double)x2/1000000.0f;
    double f  = (double)x/1000.0f;

    return p+f2+f;
}



void Utilities::addToSortedList(int *list,int n,int x){

    int i;
    for(i = n-2; i > 1 && list[i] > x; i--)
        list[i+1] = list[i];

    list[i+1] = x;
}

void Utilities::sortResPoint_AscendingId(vector<res_point*>* vc){

    struct res_point_ascending_id tmp;
    sort(vc->begin(), vc->end(), tmp);

}

void Utilities::sortResPoint_AscendingDist(vector<res_point*>* vc){

    struct res_point_ascending_dist tmp;
    sort(vc->begin(), vc->end(), tmp);
}

// erase from the vector the res_points whose dist is greater than thres
void Utilities::updateUsersInValidRange(vector<res_point*>* res, double thres){
    unsigned int i = 0;
    while(i < res->size()){
        if((*res)[i]->dist > thres)
            res->erase(res->begin()+i);
        else
            i++;
    }
}

/*
Group* Utilities::computeMyGroup(vector<res_point*>* usersInRange, int* friends, int friendsSize, res_point* p, int m){

    Group* result = new Group(p);
    priority_queue<res_point*, vector<res_point*>, res_point_ascending_dist> userFriends;

    int users = usersInRange->size();

    if(friendsSize > users){
        for(int i = 0; i< users; i++){
            if(binarySearch(friends, 0, friendsSize, (*usersInRange)[i]->id))
                userFriends.push(copy((*usersInRange)[i]));
        }
    }
    else{
        for(int i = 0; i< friendsSize; i++){
            res_point* tmp = binarySearch_res_point(usersInRange, 0, users, friends[i]);
            if(tmp != NULL)
                userFriends.push(copy(tmp));
        }
    }

    int j = 0;
    double adist = p->dist;
    while(!userFriends.empty()){
        res_point*tmp = userFriends.top();
        if(j < m){
            adist+=tmp->dist;
            j++;
        }
        result->addFriend(tmp);
        userFriends.pop()
    }
    result->adist = adist;

    return result;

}
*/

bool Utilities::binarySearch(int sortedArray[], int first, int last, int key) {
    // function:
    //   Searches sortedArray[first]..sortedArray[last] for key.
    // returns: index of the matching element if it finds key,
    //         otherwise  -(index where it could be inserted)-1.
    // parameters:
    //   sortedArray in  array of sorted (ascending) values.
    //   first, last in  lower and upper subscript bounds
    //   key         in  value to search for.
    // returns:
    //   index of key, or -insertion_position -1 if key is not
    //                 in the array. This value can easily be
    //                 transformed into the position to insert it.

    while (first <= last) {
        int mid = (first + last) / 2;  // compute mid point.
        if (key > sortedArray[mid])
            first = mid + 1;  // repeat search in top half.
        else if (key < sortedArray[mid])
            last = mid - 1; // repeat search in bottom half.
        else
            return true;     // found it. return position /////
    }
    return false;    // failed to find key
}

bool Utilities::binarySearch(vector<int>* sortedArray, int first, int last, int key) {
    // function:
    //   Searches sortedArray[first]..sortedArray[last] for key.
    // returns: index of the matching element if it finds key,
    //         otherwise  -(index where it could be inserted)-1.
    // parameters:
    //   sortedArray in  array of sorted (ascending) values.
    //   first, last in  lower and upper subscript bounds
    //   key         in  value to search for.
    // returns:
    //   index of key, or -insertion_position -1 if key is not
    //                 in the array. This value can easily be
    //                 transformed into the position to insert it.

    while (first <= last) {
        int mid = (first + last) / 2;  // compute mid point.
        if (key > (*sortedArray)[mid])
            first = mid + 1;  // repeat search in top half.
        else if (key < (*sortedArray)[mid])
            last = mid - 1; // repeat search in bottom half.
        else
            return true;     // found it. return position /////
    }
    return false;    // failed to find key
}


res_point* Utilities::binarySearch_res_point(vector<res_point*>* sortedArray, int first, int last, int key) {
    // function:
    //   Searches sortedArray[first]..sortedArray[last] for key.
    // returns: index of the matching element if it finds key,
    //         otherwise  -(index where it could be inserted)-1.
    // parameters:
    //   sortedArray in  array of sorted (ascending) values.
    //   first, last in  lower and upper subscript bounds
    //   key         in  value to search for.
    // returns:
    //   index of key, or -insertion_position -1 if key is not
    //                 in the array. This value can easily be
    //                 transformed into the position to insert it.
    while (first <= last) {
        int mid = (first + last) / 2;  // compute mid point.
        if (key > (*sortedArray)[mid]->id)
            first = mid + 1;  // repeat search in top half.
        else if (key < (*sortedArray)[mid]->id)
            last = mid - 1; // repeat search in bottom half.
        else{
            return (*sortedArray)[mid];     // found it. return position /////
        }
    }

    return NULL;    // failed to find key
}






double Utilities::computeMinimumDistance(double x1, double y1, double x2, double y2){
    return sqrt(((x1-x2)*(x1-x2))+ ((y1 - y2)*(y1 - y2)));
}

//double Utilities::computeMinimumDistance(double x, double y, double x2, double y2){

//        // pi/180 = 0.0174532925199433 (precise to double precision)

//        double dLong=(y-y2)*0.0174532925199433;
//        double dLat=(x-x2)*0.0174532925199433;

//        double aHarv= (sin(dLat/2.0)*sin(dLat/2.0))+(cos(x*0.01745329251994333)*cos(x2*0.01745329251994333)*sin(dLong/2)*sin(dLong/2));
//        double cHarv=2*atan2(sqrt(aHarv),sqrt(1.0-aHarv));

//        return 6378.137*cHarv;
//}


res_point* Utilities::copy(res_point* toBeCopied){

    res_point* rp = new res_point();
    rp->id = toBeCopied->id;
    rp->x = toBeCopied->x;
    rp->y = toBeCopied->y;
    rp->dist = toBeCopied->dist;

    return rp;
}

res_point* Utilities::createResultPoint(int id, double x, double y, double distance){

    res_point* rp = new res_point();
    rp->id = id;
    rp->x = x;
    rp->y = y;
    rp->dist = distance;

    return rp;
}

