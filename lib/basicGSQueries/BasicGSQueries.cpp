#include "../headers.h"

SimpleQueries::SimpleQueries(IGPOs *spatialIndex, ISPOs *socialGraph){
    spos = socialGraph;
    gpos = spatialIndex;
    strcpy(f, "sum");
}


SimpleQueries::~SimpleQueries(){}


//returns the mGroup of the closest user to q who has an mGroup
double SimpleQueries::getFirstmGroup(double x, double y, int m, int k){
    double maxDist = -1;
    Group* result = NULL;
    res_point* newP;
    int i = 0;

    //	gpos->clearNextNN();
    newP = gpos->getNextNN(x, y, 10);

    while(i < k && newP != NULL){
        //		cout << " - " << newP->id << " - ";
        result = getClosestFriends3(x, y, newP, m);
        if(result != NULL){
            if(maxDist < result->adist)
                maxDist = result->adist;
            i++;
        }
        newP = gpos->getNextNN(x, y, 10);
        //	else{
        //		gpos->clearNextNN();
        //		return -1;
        //	}
        //		}
    }
    //	cout << endl;
    gpos->clearNextNN();

    if(i == k)
        return maxDist;
    else
        return -1;
}



Group* SimpleQueries::computeMyGroup(vector<res_point*>* usersInRange, int* friends, int friendsSize, res_point* p,unsigned int m){

    if(friendsSize > -1){
        double adist = 0.0;
        Group* result = new Group(p, m, f);
        priority_queue <double, vector<double>, greater<double> > heap;
        int users = usersInRange->size();

        for(int i = 0; i< users; i++){
            if(util.binarySearch(friends, 0, friendsSize, (*usersInRange)[i]->id)){
                result->addFriend(util.copy((*usersInRange)[i]));
                heap.push((*usersInRange)[i]->dist);
            }
        }

        if(heap.size() >= m-1){
            for(unsigned int i = 0; i < m-1; i++){
                adist+= heap.top();
                heap.pop();
            }

            adist+= p->dist;

            result->adist = adist;
            result->updateLongestDist(-1);
        }
        else{
            result->adist = MAXDIST*m;
        }
        return result;

    }

    return NULL;

}




// get_friends intesection range 
// n friends m users in range
// complexity O(m*sqrt(n))
Group* SimpleQueries::getMyFriendsInRange1(double x, double y, double radius, res_point* user){

    //Utilities ut;
    //struct timeval start, end;
    Group* result = new Group(user, MAXGROUPSIZE, f);
    int* friends;
    unsigned int friendsSize;

    spos->getFriends(user->id, friends, friendsSize);

    vector<res_point*>* usersInRange = gpos->getRange(x, y, radius);

    for(unsigned int i=0; i < usersInRange->size(); i++){
        res_point* tmp = (*usersInRange)[i];

        if(util.binarySearch(friends, 0, friendsSize, tmp->id)){
            result-> addFriend(util.copy(tmp));
        }
    }

    // memory deallocation
    free(friends);
    for(unsigned i = 0; i < usersInRange->size(); i++)
        delete usersInRange->at(i);
    delete usersInRange;

    return result;
}


// range and are friends
Group* SimpleQueries::getMyFriendsInRange2a(double x, double y, double radius, res_point* user){

    Group* result = new Group(user, MAXGROUPSIZE, f);
    vector<res_point*>* usersInRange = gpos->getRange(x, y, radius);

    for(int i=0; i < (int)usersInRange->size(); i++){
        res_point* u = (*usersInRange)[i];
        if(spos->areFriends(user->id, u->id)){
            result-> addFriend(util.copy(u));
        }
    }

    for(unsigned i = 0; i < usersInRange->size(); i++)
        delete usersInRange->at(i);
    delete usersInRange;

    return result;
}

// range and are friends (we already have computed the range)
Group* SimpleQueries::getMyFriendsInRange2b(vector<res_point*>* seen_users, res_point* user){
    Group* result = new Group(user, MAXGROUPSIZE, f);

    // similar to the above approach
    // but now we have already computed the users in the range

    for(int i=0; i < (int)seen_users->size(); i++){
        res_point* u = (*seen_users)[i];
        if(spos->areFriends(user->id, u->id)){
            result-> addFriend(util.copy(u));
        }
    }

    return result;
}

// getfriends and get user location
Group* SimpleQueries::getMyFriendsInRange3(double x, double y, double radius, res_point* user){

    Group* result = new Group(user, MAXGROUPSIZE, f);
    int* friends;
    unsigned int friendsSize;

    spos->getFriends(user->id, friends, friendsSize);

    double userLocation[2];
    // for each friend check if his location is in range
    for(unsigned int i=0; i< friendsSize; i++){
        gpos->getLocation(friends[i], userLocation);
        if(userLocation[0]!=-1000){ // is not null
            double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
            if(dist <= radius){
                res_point* rp = util.createResultPoint(friends[i], userLocation[0], userLocation[1], dist);
                result->addFriend(rp);
            }
        }
    }

    delete friends;

    return result;
}



// get friends, get next user and check if they are friends localy
Group* SimpleQueries::getClosestFriends1(double x, double y, res_point* user, unsigned int m, int incrStep){
    Group* result = new Group(user, m, f);
    unsigned int count = 0;
    res_point* newP;
    int* friends;
    unsigned int friendsSize;

    spos->getFriends(user->id, friends, friendsSize);

    if(friendsSize >= m-1){
        while(count < m-1){
            newP = gpos->getNextNN(x, y, incrStep);
            //if they are friends, then add it to the group and increase count
            if(newP != NULL){

                if(util.binarySearch(friends, 0, friendsSize, newP->id)){
                    result-> addFriend(util.copy(newP));
                    count++;
                    //	cout << "AreFriends(" << user->id << ", " << newP->id << ")\n";
                }
            }
            else{
                delete result;
                gpos->clearNextNN();
                return NULL;
            }
        }

        delete friends;
        gpos->clearNextNN();

        if(count == m-1)
            return result;

    }
    gpos->clearNextNN();
    delete result;
    return NULL;

}


// get next user and check if they are friends, keep doing it till you found m friends
Group* SimpleQueries::getClosestFriends2(double x, double y, res_point* user, unsigned int m, int incrStep){

    Group* result = new Group(user, m, f);
    unsigned int count = 0;
    res_point* newP;

    newP = gpos->getNextNN(x, y, incrStep);

    while(count < m-1 && newP != NULL){

        if(spos->areFriends(user->id, newP->id)){
            result-> addFriend(util.copy(newP));
            count++;
        }

        newP = gpos->getNextNN(x, y, incrStep);

    }

    gpos->clearNextNN();

    if(count == m-1)
        return result;
    else{
        delete result;
        return NULL;
    }
}




//get friends, for each one of them compute the mindist and return the k closest
Group* SimpleQueries::getClosestFriends3(double x, double y, res_point* user,unsigned int m){

    Group* result = new Group(user, m, f);
    priority_queue <res_point*, vector<res_point*>, res_point_ascending_dist >* heap = new priority_queue <res_point*, vector<res_point*>, res_point_ascending_dist >();
    int* friends;
    unsigned int friendsSize=0;
    double userLocation[2];

    spos->getFriends(user->id, friends, friendsSize);

    if(friendsSize >= m-1){

        for(unsigned int i=0; i< friendsSize; i++){
            gpos->getLocation(friends[i], userLocation);
            if(userLocation[0] != -1000){
                res_point* rp = util.createResultPoint(friends[i], userLocation[0], userLocation[1], util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]));
                heap->push(rp);
            }
        }

        if(heap->size() >= m-1){
            for(unsigned int i = 0; i < m-1; i++){
                res_point* p = heap->top();
                result->addFriend(p);
                heap->pop();
            }

            while(!heap->empty()) {
                delete heap->top();
                heap->pop();
            }

            delete friends;
            delete heap;

            result->updateLongestDist(-1);

            return result;
        }
    }

    delete result;
    return NULL;
}



//get friends, for each one of them compute the mindist and return the k closest
Group* SimpleQueries::getClosestFriends3(double x, double y, res_point* user, unsigned int m, char* function){

    Group* result = new Group(user, m, function);
    priority_queue <res_point*, vector<res_point*>, res_point_ascending_dist >* heap = new priority_queue <res_point*, vector<res_point*>, res_point_ascending_dist >();
    int* friends;
    unsigned int friendsSize=0;
    double userLocation[2];

    spos->getFriends(user->id, friends, friendsSize);

    if(friendsSize >= m-1){

        for(unsigned int i=0; i< friendsSize; i++){
            gpos->getLocation(friends[i], userLocation);
            if(userLocation[0] != -1000){
                res_point* rp = util.createResultPoint(friends[i], userLocation[0], userLocation[1], util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]));
                heap->push(rp);
            }
        }

        if(heap->size() >= m-1){
            for(unsigned int i = 0; i < m-1; i++){
                res_point* p = heap->top();
                result->addFriend(p);
                heap->pop();
            }

            while(!heap->empty()) {
                delete heap->top();
                heap->pop();
            }

            delete friends;
            delete heap;

            result->updateLongestDist(-1);

            return result;
        }
    }

    delete result;
    return NULL;
}




// Restricted, returns the friends of user in a specific range, but the aggregate distance of the group is
// the sum of distances of user and his m-1 closest friends to (x, y). 

Group* SimpleQueries::getMyFriendsInRange3(double x, double y, double radius, res_point* user,unsigned int m){

    Group* result = new Group(user, m, f);
    priority_queue <double, vector<double>, greater<double> > heap;// = new priority_queue <double, vector<double>, greater<double> >();

    int* friends;
    unsigned int friendsSize = 0;
    double adist = 0;
    double userLocation[2];
    spos->getFriends(user->id, friends, friendsSize);

    if(friendsSize > 0){
        for(unsigned int i=0; i< friendsSize; i++){
            gpos->getLocation(friends[i], userLocation);

            if(userLocation[0] != -1000 ){
                double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
                if(dist <= radius){
                    res_point* rp = util.createResultPoint(friends[i], userLocation[0], userLocation[1], dist);
                    result->addFriend(rp);
                    heap.push(dist);
                }
            }
        }

        if(heap.size() >= m-1){
            for(unsigned int i = 0; i < m-1; i++){
                adist+= heap.top();
                heap.pop();
            }

            adist+= user->dist;

            result->adist = adist;
            result->updateLongestDist(-1);
        }
        else
            result->adist = MAXDIST*m;

        free(friends);
        heap = priority_queue <double, vector<double>, greater<double> >();
        return result;

    }
    return NULL;
}
