#include "../headers.h"

topkUsers::topkUsers(IGPOs *spatialIndex, ISPOs *socialGraph, SimpleQueries *simpleQueries){
	spos = socialGraph;
	gpos = spatialIndex;
	sq = simpleQueries;

	usersChecked = 0;
	threshold = 0.0;
	current_dist = 0.0;

	computeLBTime = 0.0;
	seen_usersVEC = new vector<res_point*>();
}

topkUsers::~topkUsers(){}

int topkUsers::getNumOfUsersChecked(){
	return usersChecked;
}

double topkUsers::getThreshold(){
	return threshold;
}

double topkUsers::getDensity(Group* gp){
	double triangles=0;

	vector<res_point*>* vectorFriends = new vector<res_point*>();
	Group* newG = new Group(gp);
	priority_queue<res_point*, vector<res_point*>, res_point_ascending_dist>* tmp = newG->friends;
	int size = gp->friends->size();
	//    cout << "friends size = "<<size<<endl;
	while (!tmp->empty())
	{
		vectorFriends->push_back(tmp->top());
		tmp->pop();
	}

	for(unsigned int i = 0; i <vectorFriends->size();i++){
		res_point* a = vectorFriends->at(i);
		for(unsigned int j = i+1; j <vectorFriends->size(); j++){
			res_point* b = vectorFriends->at(j);
			if(spos->areFriends(a->id,b->id))
			triangles++;
		}
	}

	//    cout<<"actual triangles = "<<triangles<<endl;
	double maxTriangles = size*(size-1)/2;
	//    cout << "max triangles possible ="<<maxTriangles<<endl;
	if(triangles==0)
	return 0;
	else
	return triangles/maxTriangles;
}


double topkUsers::getOverlapInTopK(vector<int>* result_vec){
	double overlap = 0;
	
	unordered_set<int> result_set ;
	for(auto it = result_vec->begin(); it < result_vec->end();it++){
		result_set.insert(*it);
	}
	
	int* friends;
	unsigned int friendsSize;
	for(auto it = result_vec->begin(); it < result_vec->end();it++){
		spos->getFriends(*it, friends, friendsSize);
		
		for(unsigned int j = 0; j < friendsSize; j++){
			if(result_set.find(friends[j]) != result_set.end()){
				overlap++;
			}
		}
	}
	return overlap / (result_vec->size()*(result_vec->size()-1));
}

double topkUsers::getOverlapInFriends(vector<int>* result_vec){
	double overlap = 0;
	for(unsigned int i = 0; i < result_vec->size(); i++){
		int* friends;
		unsigned int friendsSize;
		spos->getFriends(result_vec->at(i), friends, friendsSize);
		if(friendsSize == 0) continue;
		unordered_set<int> friends_set;
		for(unsigned int f = 0; f < friendsSize; f++){
			friends_set.insert(friends[f]);
		}
		for(unsigned int j = i + 1; j < result_vec->size(); j++){
			int intersection = 0;
			spos->getFriends(result_vec->at(j), friends, friendsSize);
			if(friendsSize == 0) continue;
			for(unsigned int f = 0; f < friendsSize; f ++){
				auto it_set = friends_set.find(friends[f]);
				if(it_set != friends_set.end()){
					intersection++;
				}
			}
			
			double min_friends ;
			if(friendsSize < friends_set.size())
				min_friends = friendsSize;
			else
				min_friends = friends_set.size();
				
			if(intersection == 0) continue;
			overlap += (intersection / (double) min_friends);
		}
	}
	
	double combinations = (result_vec->size()*(result_vec->size()-1))/2;
	return overlap / combinations;
}

double topkUsers::getInterDivUnion(vector<int>* result_vec){
	int intersection = 0;
	int total = 0;
	vector<int> array;
	
	for(unsigned int i = 0; i < result_vec->size(); i++){
		int* friends;
		unsigned int friendsSize;
		spos->getFriends(result_vec->at(i), friends, friendsSize);
		if(friendsSize == 0) continue;
		
		total+=friendsSize;
		for(unsigned int i = 0; i< friendsSize ; i++){
			array.push_back(friends[i]);
		}
		delete friends;
	}
	
	
	
	std::sort(array.begin(), array.end());
	for (size_t i = 1; i < array.size(); ++i) {
		if (array.at(i) == array.at(i - 1))
			intersection++;
	}
	cout<<"Total: "<<total<<" Array Size: "<<array.size()<<" Intersection: "<<intersection<<endl;
	
	return intersection/(double)(total - intersection);
}



/*
returns k users in the weight adjusted radius with the largest
score, obtained through the Linear Combination Algorithm.
*/
priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkUsers::LCA(double x, double y, int k, double w, double r){

	Group* newG;
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	double best_score = -1;
	int heap_size = 0;
	vector<res_point*>* usersInValidRange;
	usersChecked =0;
	threshold =0;

	double radius = w*r/(1-w);

	usersInValidRange = gpos->getRange(x, y, radius);
	threshold = radius;
	
	//cout<<"Weight adjusted radius = "<<radius<<" | "<<radius*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
	//cout<<"Users in radius = "<<radius<<" => "<<(int)usersInValidRange->size()<<endl;

	usersChecked=usersInValidRange->size();
	res_point *tmp;
	int* friends;
	unsigned int friendsSize;

	for(int i = 0; i < (int)usersInValidRange->size(); i++){
		res_point* newP = (*usersInValidRange)[i];
		//cout << "i = " << i << "\t id = " << newP->id << endl;
		spos->getFriends(newP->id, friends, friendsSize);
		newG = new Group(newP);

		for(int j = 0; j < (int)usersInValidRange->size(); j++){
			tmp = (*usersInValidRange)[j];

			if(util.binarySearch(friends, 0, friendsSize, tmp->id)){
				newG->addFriend(tmp);
			}
		}

		newG->score = w*newG->size()/MAXSC+(1-w)*(1-(newG->adist/(MAXDIST*MAXSC)));

		if(newG->score >= best_score){
			heap->push(new Group(newG));
			heap_size++;

			if(heap_size > k){
				heap->pop();
				heap_size--;
				best_score = heap->top()->score;
			}
			else if(heap_size == k){
				best_score = heap->top()->score;
			}
		}
	}
	
	
	if( heap_size < k ){
		vector<res_point*>* kNN = gpos->getkNN(x, y, k);
		res_point* nearestP;

		for (int i = heap_size; i < k ; i++){
			nearestP = kNN->at(i);
			Group* gp = new Group(nearestP);
			gp->score=0;
			heap->push(gp);
		}
	}

	return heap;
}


Group* topkUsers::RC_BoundComputation(res_point* user, vector<res_point*>* seen_users, double w){

	//cout << "start of score module social = " << social << "   user = " << user->id << endl;
	Group* newG = new Group(user);
	
	double avgDist = user->dist/(1-w);
	double totaldist = user->dist;
	int size = 1;

	int minSize = seen_users->size();
	if(MAXSC < minSize)
		minSize = MAXSC;
	
	int counter = 0;
	vector<res_point*>::iterator i;
	//cout<<"MINSIZZE = "<<minSize<<endl;
	for(i = seen_users->begin(); i != seen_users->end() ; i++){
		//                cout << "x = " << x << endl;
		//                cout << "id = " << (*i)->id << endl;
		//                cout << "dist = " << (*i)->dist << endl;
		//                cout<<"(*i)->dist <= avgDist= "<< ((*i)->dist <= avgDist ? "True" : "False") <<endl;
		//cout<<counter<<" \t avgDist ="<< avgDist<<" | i->dist = "<<(*i)->dist<< " | user->dist = "<<user->dist;
		//&& counterMap[(*i)->id]>0
		if(counter < minSize){
			if( (*i)->dist <= avgDist){
				map<int, int>::iterator MapIter = counterMap.find((*i)->id);
				if(MapIter->second > 0){
					newG->addFriend(*i);
					totaldist += (*i)->dist;
					size++;
					avgDist = totaldist/(size-w);
					//cout<<" => added ("<< (*i)->id <<")";
				}
			}
			else{
				counter = -99;
				break;
			}
			counter++;
		}
		else{
			break;
		}
		//cout<<endl;
	}
	int j = MAXSC - seen_users->size();
	if(j > 0 || counter == -99){
		for(int j = MAXSC - seen_users->size(); j > 0;j--){
				if(user->dist <= avgDist){
					newG->addFriend(user);
					totaldist += user->dist;
					size++;
					avgDist = totaldist/(size-w);
				}
				else{
					break;
				}
		}
	}
	
	newG->score = (newG->size()-w)/newG->adist;
	
	//    cout << "end of score module with score = "<<newG->score << endl;
	return newG;
}

// Computes the LB for an unseen user. In the best case, his group contains the m-1 closest users to q.

Group* topkUsers::RC_ScoreComputation(double x, double y, res_point* user,vector<res_point*>* friendVEC, double w){

	//cout << "start of score module social = " << social << "   user = " << user->id << endl;
	Group* newG = new Group(user);
	
	double avgDist = user->dist/(1-w);
	double totaldist = user->dist;
	int size = 1;

	vector<res_point*>::iterator i;
	//cout<<"size of friends for user "<<user->id<<" = "<< friendVEC->size()<<endl;
	for( i=friendVEC->begin() ; i < friendVEC->end(); i++){ 

		//cout<<" \t avgDist ="<< avgDist<<" | dist = "<<(*i)->dist<< " | user->dist = "<<user->dist<<endl;
		if( (*i)->dist <= avgDist){
			newG->addFriend(*i);
			totaldist += (*i)->dist;
			size++;
			avgDist = totaldist/(size-w);
		}
		else
			break;
		
	}
	
	newG->score = (newG->size()-w)/newG->adist;

	//    cout << "end of score module with score = "<<newG->score << endl;
	return newG;
}

priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkUsers::RC(double x, double y,unsigned int k, int incrStep, double w){

	//struct timeval startBound, endBound, startScoring, endScoring, startGF, endGF, startA, endA, startNN,endNN;
	//double sumBound=0, sumScoring =0, sumGF=0, sumA=0;
	
	counterMap.clear();
	double best_score = -1;
	double lb = 999999;
	res_point* newP;
	Group* tmpG;
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	unsigned int heap_size = 0;
	vector<res_point*>* seen_users = new vector<res_point*>();
	Group* T_group;
	// gettimeofday(&startNN, NULL);
	newP = gpos->getNextNN(x, y, incrStep);
	//newP = gpos->getNextNN(x, y);
	seen_users->push_back(newP);
	usersChecked =0;
	threshold =0;

	int* friends;
	unsigned int friendsSize = 0 ;
	// gettimeofday(&endNN, NULL);
	// cout<<"Total Time for getkNN= "<<print_time(startNN, endNN)/1000<<" ms"<<endl;

	while(lb > best_score && newP != NULL && heap->size() <= k){
		//cout << "user = " << newP->id << "\t dist = " << newP->dist << "\t best_score = " << best_score << endl;

		vector<res_point*>* friendVEC = new vector<res_point*>();		
		// gettimeofday(&startGF, NULL);
		
		// if(found)
		//then
		spos->getFriends(newP->id, friends, friendsSize);
		// pair<int*, int>* fPair = new  pair<int*, int>(friends,friendsSize);
		// friendList.insert(make_pair(newP->id,fPair));

		for(unsigned int j=0; j< friendsSize; j++){ 
			double userLocation[2];
			gpos->getLocation(friends[j], userLocation);
			if(userLocation[0]!=-1000){ // is not null
				double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				res_point* rp = util.createResultPoint(friends[j], userLocation[0], userLocation[1], dist);
				friendVEC->push_back(rp);
			}
		}
		util.sortResPoint_AscendingDist(friendVEC);
		
		vector<res_point*>::iterator i; int localcounter = friendsSize;
		map<int, int>::iterator MapIter;
		//cout<<"newP->id = "<<newP->id<<" Friendsize = "<<friendsSize<<endl;
		for( i=friendVEC->begin() ; i < friendVEC->end(); i++){ 
			
			if( (newP->dist - (*i)->dist) > BOUNDARY_ERROR ){
				localcounter --;
				MapIter = counterMap.find((*i)->id);
				//cout<<"\t Before counterMap[(*i)->id] ("<<(*i)->id<<") = "<<MapIter->second <<endl;
				MapIter->second = MapIter->second - 1;
				//cout<<"\t After counterMap[(*i)->id] ("<<(*i)->id<<") = "<<MapIter->second <<endl;
			}
			else
				break;
		}
		//cout<<"newP->id = "<<newP->id<<" distance = "<<newP->dist*EARTH_CIRCUMFERENCE/360<<" Friendsize = "<<friendsSize<<" localcounter = "<<localcounter<<endl;
		//cout<<"======================"<<endl;
		counterMap.insert(make_pair(newP->id,localcounter));
		
		
		// gettimeofday(&endGF, NULL);
		// sumGF+=print_time(startGF, endGF);
		
		// gettimeofday(&startScoring, NULL);
		tmpG = RC_ScoreComputation(x, y, newP, friendVEC, w);
		// gettimeofday(&endScoring, NULL);
		// sumScoring+=print_time(startScoring, endScoring);
		//        cout << "tentative best score = " << tmpG->score << endl;

		if(tmpG!=NULL){
			//		cout << "ok 1 !" << endl;
			if(tmpG->score > best_score){
				//	cout << "ok 2 !" << endl;
				heap->push(tmpG);
				heap_size++;
				//	cout << "ok 3 !" << endl;

				if(heap_size > k){
					heap->pop();
					heap_size--;
					best_score = heap->top()->score;
				}
				else if(heap_size == k){
					best_score = heap->top()->score;
				}
			}
			//            cout << "Best score = " << best_score << endl;
		}
		
		if(heap_size == k){
			
			// gettimeofday(&startBound, NULL);
			T_group = RC_BoundComputation(newP, seen_users, w);
			// gettimeofday(&endBound, NULL);
			// sumBound+=print_time(startBound, endBound);
			
			lb = T_group->score;

			 //          cout<<"T_score is = "<<lb<<endl;
			delete T_group;
		}

		if(lb < best_score){
			//            cout <<"--------------------------------------" << endl;
			//            cout<<"UCA completed!" << endl;
			//cout<<"Radius of search = "<<newP->dist <<" | "<<newP->dist*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
			//            cout<<"User Id of terminating user = "<<newP->id<<endl;
			//            cout <<"--------------------------------------" << endl;
			threshold = newP->dist;
			break;
		}
		
		
		// gettimeofday(&startA, NULL);
	
		newP = gpos->getNextNN(x, y, incrStep);
		//newP = gpos->getNextNN(x, y);
		
		if(newP != NULL){
			seen_users->push_back(newP);
		}
		
		// gettimeofday(&endA, NULL);
		// sumA+=print_time(startA, endA);
		 //      cout <<"--------------------------------------" << endl;
		usersChecked++;
	}
	// cout<<"Total Time for computing score= "<<sumScoring/1000<<" ms"<<endl;
	// cout<<"Total Time for computing bound = "<<sumBound/1000<<" ms"<<endl;
	// cout<<"Total Time for computing GetFriends = "<<sumGF/1000<<" ms"<<endl;
	// cout<<"Total Time for getNextNN = "<<sumA/1000<<" ms"<<endl;
	gpos->clearNextNN();
	return heap;
}


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkUsers::HGS(double x, double y, int k, double d){

	Group* newG;
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	double best_score = -1;
	int heap_size = 0;
	usersChecked =0;
	threshold =0;

	set<res_point*, res_point_ascending_id>* initialCandidatePoints = gpos->getSetRange(x, y, d);
	set<res_point*, res_point_ascending_id>* candidatePoints = new set<res_point*, res_point_ascending_id>();

	//    set<res_point*, res_point_ascending_id>::iterator it;
	//    for(it =setRange->begin(); it != setRange->end();it++)
	//        cout << "id = " << (*it)->id << " x = " << (*it)->x << " y = " << (*it)->y << " dist = " << (*it)->dist << endl;

	//    res_point* old_res = (*setRange->rbegin());

	//    res_point* newP = new res_point();
	//    newP->id = 22624058;
	//    newP->x = -74.0822;
	//    newP->y = 40.8821;
	//    newP->dist = 0.0132306462427177;
	//    setRange->insert(newP);

	//   // setRange->insert(old_res);

	//    cout << "---------------------------------------------------" << endl;

	//    for(it =setRange->begin(); it != setRange->end();it++)
	//        cout << "id = " << (*it)->id << " x = " << (*it)->x << " y = " << (*it)->y << " dist = " << (*it)->dist << endl;


	//   usersInValidRange = gpos->getRange(x, y, d);
	//cout<<"Users in radius = "<<d<<" => "<<(int)usersInValidRange->size()<<endl;

	//   vector<res_point*>* candidatePointsDup = new vector<res_point*>();
	//   vector<res_point*>* candidatePoints = new vector<res_point*>();

	//    for(int i = 0; i < (int)usersInValidRange->size(); i++){
	//        candidatePointsDup->push_back((*usersInValidRange)[i]);
	//    }

	int* friends;
	unsigned int friendsSize = 0;

	set<res_point*, res_point_ascending_id>::iterator setIter;
	//    cout<<"initialCandidatePoints size =" <<initialCandidatePoints->size()<<endl;
	for(setIter = initialCandidatePoints->begin(); setIter != initialCandidatePoints->end() ; setIter++){
		res_point* newP = *setIter;
		//        cout<< "\t id = " << (*setIter)->id << endl;
		spos->getFriends(newP->id, friends, friendsSize);

		double userLocation[2];
		for(unsigned int j=0; j< friendsSize; j++){  // ------------------------- you can check if he is in tmp or candidate points already
			gpos->getLocation(friends[j], userLocation);
			if(userLocation[0]!=-1000){ // is not null
				double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				res_point* rp = util.createResultPoint(friends[j], userLocation[0], userLocation[1], dist);
				candidatePoints->insert(rp);
			}
		}
		delete friends;
	}
	//    cout<<"----------------------------"<<endl;
	//    cout<<"candidatePoints size with duplicates = "<<candidatePointsDup->size()<<endl;
	//    cout<<"All candidates collected, now removing duplicates, if any."<<endl;

	//  set<res_point*, res_point_ascending_dist >* finalCandidates = new set<res_point*, res_point_ascending_dist >();

	candidatePoints->insert(initialCandidatePoints->begin(), initialCandidatePoints->end());
	//  finalCandidates->insert(candidatePoints->begin(), candidatePoints->end());
	usersChecked = candidatePoints->size();
	//    cout<<"candidatePoints size =" <<candidatePoints->size()<<endl;

	//    for(setIter = candidatePoints->begin(); setIter != candidatePoints->end() ; setIter++){
	//        cout<< "\t id = " << (*setIter)->id << endl;
	//    }

	//remove duplicates in case a person is friends with more than one candidates
	//util.sortResPoint_AscendingId(candidatePointsDup);
	//    candidatePoints->clear();

	//    for (unsigned int i = 0; i < candidatePointsDup->size(); i++) {
	//        cout << "i = " << i << "\t id = " << ((*candidatePointsDup)[i])->id << endl;
	//    }

	//  for (unsigned int i = 0; i < candidatePointsDup->size(); i++) {
	//      res_point* duplicate1 = (*candidatePointsDup)[i];
	//        res_point* duplicate2 = (*candidatePointsDup)[i-1];
	//        if (i > 0 && duplicate1->id == duplicate2->id)
	//            continue;
	//        candidatePoints->push_back(duplicate1);
	//    }

	//cout <<"candidatePoints size without duplicates = "<<candidatePoints->size()<<endl;
	//    for (unsigned int i = 0; i < candidatePoints->size(); i++) {
	//        cout << "i = " << i << "\t id = " << ((*candidatePoints)[i])->id << endl;
	//    }

	friendsSize = 0;


	//util.sortResPoint_AscendingDist(candidatePoints);

	for(setIter = candidatePoints->begin(); setIter != candidatePoints->end() ; setIter++){
		res_point* newP = (*setIter);
		vector<res_point*>* F = new vector<res_point*>();
		//        cout << "CandidatePoints final for loop = " << (*setIter)->id << endl;
		spos->getFriends(newP->id, friends, friendsSize);
		newG = new Group(newP);

		double userLocation[2];
		for(unsigned int j=0; j< friendsSize; j++){
			gpos->getLocation(friends[j], userLocation);//consider implementing getMuliplelocations in consideration of nework processing time
			if(userLocation[0]!=-1000){ // is not null
				double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				res_point* rp = util.createResultPoint(friends[j], userLocation[0], userLocation[1], dist);
				F->push_back(rp);
			}
		}

		util.sortResPoint_AscendingDist(F);
		//        for(unsigned int p =0; p < F->size(); p++)   {
		//            cout << "F points i = " << p << "\t id = " << ((*F)[p])->id <<"  dist = "<<((*F)[p])->dist<< endl;
		//        }

		unsigned int index = 1;
		unsigned int newP_valid = 1;
		double value = d;

		if(F->size()>0) {
			while((((*F)[index-1])->dist <= value)){

				//cout<<"index = "<<index<<" | value = "<<value<<" | "<<value*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
				if(newP->dist <= value)
				newP_valid = index;

				newG->addFriend((*F)[index-1]);
				index++;
				if((index-1)==F->size())
				break;

				value += (d+(index-1)*d)/pow(2,index-1);
				//value += d/pow(2,index-1);
				//value += d/(index);
				//value=index*d;

			}
			newG->score=newP_valid;

		}



		if(newG->score >= best_score){
			heap->push(new Group(newG));
			heap_size++;

			if(heap_size > k){
				heap->pop();
				heap_size--;
				best_score = heap->top()->score;
			}
			else if(heap_size == k){
				best_score = heap->top()->score;
			}
		}

		if(threshold<value){
			threshold = value;
		}
	}

	//incorrect FIX LATER

	if( heap_size < k ){
		vector<res_point*>* kNN = gpos->getkNN(x, y, k);
		res_point* nearestP;

		for (int i = heap_size; i < k ; i++){
			nearestP = kNN->at(i);
			heap->push(new Group(nearestP));
		}
	}
	//    cout <<"----- ---------------------------------" << endl;
	//    cout << "HGS Completed" << endl;
	//    cout <<"--------------------------------------" << endl;
	return heap;
}

priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkUsers::HGS2(double x, double y, int k, double d){

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	double best_score = -1;
	int heap_size = 0;
	usersChecked =0;
	threshold =0;

	map<int, Group*> groups;
	set<res_point*, res_point_ascending_id>::iterator setIter;
	
	set<res_point*, res_point_ascending_id>* initialCandidatePoints = gpos->getSetRange(x, y, d);
	cout<<"initialCandidatePoints size =" <<initialCandidatePoints->size()<<endl;
	
	// for(setIter = initialCandidatePoints->begin(); setIter != initialCandidatePoints->end() ; setIter++){
	// cout<< "\t id = " << (*setIter)->id << endl;
	// }
	
	
	set<res_point*, res_point_ascending_id>* candidatePoints = new set<res_point*, res_point_ascending_id>();

	/*
	//    set<res_point*, res_point_ascending_id>::iterator it;
	//    for(it =setRange->begin(); it != setRange->end();it++)
	//        cout << "id = " << (*it)->id << " x = " << (*it)->x << " y = " << (*it)->y << " dist = " << (*it)->dist << endl;

	//    res_point* old_res = (*setRange->rbegin());
	//    res_point* newP = new res_point();
	//    newP->id = 22624058;
	//    newP->x = -74.0822;
	//    newP->y = 40.8821;
	//    newP->dist = 0.0132306462427177;
	//    setRange->insert(newP);
	//   // setRange->insert(old_res);
	//    cout << "---------------------------------------------------" << endl;
	//    for(it =setRange->begin(); it != setRange->end();it++)
	//        cout << "id = " << (*it)->id << " x = " << (*it)->x << " y = " << (*it)->y << " dist = " << (*it)->dist << endl;
	*/
	
	int* friends;
	unsigned int friendsSize = 0;

	
	for(setIter = initialCandidatePoints->begin(); setIter != initialCandidatePoints->end() ; setIter++){
		res_point* newP = *setIter;
		//        cout<< "\t id = " << (*setIter)->id << endl;
		spos->getFriends(newP->id, friends, friendsSize);

		double userLocation[2];
		for(unsigned int j=0; j< friendsSize; j++){  // ------------------------- you can check if he is in tmp or candidate points already
			gpos->getLocation(friends[j], userLocation);
			if(userLocation[0]!=-1000){ // is not null
				double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				res_point* rp = util.createResultPoint(friends[j], userLocation[0], userLocation[1], dist);
				candidatePoints->insert(rp);
			}
		}
		delete friends;
	}

	candidatePoints->insert(initialCandidatePoints->begin(), initialCandidatePoints->end());
	usersChecked = candidatePoints->size();
	cout<<"candidatePoints size =" <<candidatePoints->size()<<endl;

	// for(setIter = candidatePoints->begin(); setIter != candidatePoints->end() ; setIter++){
	// cout<< "\t id = " << (*setIter)->id << endl;
	// }
	
	friendsSize = 0;
	unsigned int index = 2;
	// unsigned int newP_valid = 1;
	double value = d;
	cout <<"starting DO WHILE"<<endl;
	
	do{
		cout<<"current distance value = "<<value<<endl;
		set<res_point*, res_point_ascending_id>* candidatePrimeInitial = gpos->getSetRange(x, y, value);
		cout<<"get range in index size =" <<candidatePrimeInitial->size()<<endl;
		
		value += (d+(index-1)*d)/pow(2,index-1);
		//value += d/pow(2,index-1);
		//value += d/(index);
		//value=index*d;
		cout<<"next distance value = "<<value<<endl;

		if(threshold<value){
			threshold = value;
		}
		
		set<res_point*, res_point_ascending_id>* candidatePrime = gpos->getSetRange(x, y, value);
		cout<<"get range in index + 1 size =" <<candidatePrime->size()<<endl;
		
		for(setIter = candidatePrimeInitial->begin(); setIter != candidatePrimeInitial->end() ; setIter++){
			candidatePrime->erase(*setIter);
		}
		cout<<"candidatePrime size =" <<candidatePrime->size()<<endl;
		
		set<res_point*, res_point_ascending_id>* userSet (candidatePrime);
		
		for(setIter = userSet->begin(); setIter != userSet->end() ; setIter++){
			res_point* newP = *setIter;
			//        cout<< "\t id = " << (*setIter)->id << endl;
			spos->getFriends(newP->id, friends, friendsSize);

			double userLocation[2];
			for(unsigned int j=0; j< friendsSize; j++){  // ------------------------- you can check if he is in tmp or candidate points already
				gpos->getLocation(friends[j], userLocation);
				if(userLocation[0]!=-1000){ // is not null
					double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
					res_point* rp = util.createResultPoint(friends[j], userLocation[0], userLocation[1], dist);
					//candidatePoints->insert(rp);
					candidatePrime->insert(rp);
				}
			}
			delete friends;
		}
		
		//cout<<"candidatePrime size with friends =" <<candidatePrime->size()<<endl;
		
		//cout<<"candidatePoints size =" <<candidatePoints->size()<<endl;
		
		set<res_point*, res_point_ascending_id>* candidateIntersection = new set<res_point*, res_point_ascending_id>();;
		
		//find intersection of candidatePrime and candidatePoints
		
		for(setIter = candidatePoints->begin(); setIter != candidatePoints->end() ; setIter++){
			res_point* newP = (*setIter);
			if (candidatePrime->find(newP)!=candidatePrime->end()){
				candidateIntersection->insert(newP);
			}
		}
		
		//cout<<"candidateIntersection / (candidatePrime INTER candidatePoints) size =" <<candidateIntersection->size()<<endl;
		
		//FOR EACH C BELONGING TO CANDIDATEOUTERPOINTS AND NOT BELONGING TO CANDIDATES
		/*
		set<res_point*, res_point_ascending_id>::iterator outerSetIterBegin;
		set<res_point*, res_point_ascending_id>::iterator outerSetIterEnd;
		set<res_point*, res_point_ascending_id>::iterator innerSetIterBegin;
		set<res_point*, res_point_ascending_id>::iterator innerSetIterEnd;
		
		
		if(candidatePoints->size() > candidateOuterPoints->size()){
			outerSetIterBegin = candidateOuterPoints->begin();
			outerSetIterEnd = candidateOuterPoints->end();
			innerSetIterBegin = candidatePoints->begin();
			innerSetIterEnd = candidatePoints->end();
		}
		else {
			innerSetIterBegin = candidateOuterPoints->begin();
			innerSetIterEnd = candidateOuterPoints->end();
			outerSetIterBegin = candidatePoints->begin();
			outerSetIterEnd = candidatePoints->end();
		}
		*/
		
		for(setIter = candidatePoints->begin(); setIter != candidatePoints->end() ; setIter++){
			res_point* newP = (*setIter);
			if (candidateIntersection->find(newP)==candidateIntersection->end()){
				//cout <<"c BELONGS TO candidatePrime AND NOT DELONGSTO candidates" <<endl;
				Group* newG = NULL;
				
				if((groups.find(newP->id))==groups.end() ){
					newG  = new Group(newP);
					newG->score=index - 1;
					groups.insert(make_pair(newP->id, newG));
				}
				else{
					newG = (*(groups.find(newP->id))).second;
					newG->score=index - 1;
				}
				
				if(newG->score >= best_score){
					heap->push(new Group(newG));
					heap_size++;

					if(heap_size > k){
						heap->pop();
						heap_size--;
						best_score = heap->top()->score;
					}
					else if(heap_size == k){
						best_score = heap->top()->score;
					}
				}
			}
		}

		index = index+1;
		candidatePoints->clear();
		candidatePoints->insert(candidateIntersection->begin(),candidateIntersection->end());

	}while(!candidatePoints->empty()) ;

	//    cout <<"----- ---------------------------------" << endl;
	//    cout << "HGS Completed" << endl;
	//    cout <<"--------------------------------------" << endl;
	return heap;
}


// class my_pair{

// private:
	// int id;
	// double score;

// public:
	// my_pair(int id, double score): id(id), score(score){}

	// int getId() const {return id;}
	// double getScore() const {return score;}

// };

// struct pair_comparator_descending{

	// bool operator()(const my_pair  &x, const my_pair  &y){
		// return y.getScore() < x.getScore();
	// }
// };

struct int_comparator_descending{

	bool operator()(const int &__x, const int &__y){
		return __y < __x;
	}
};

// struct MatchPairSecond
// {
	// MatchPairSecond(double &y) : x(y) {}

	// bool operator()(const my_pair &val) {
		// return (val.getScore() == x);
	// }

	// double &x;
// };


/*
double topkUsers::ScoreModuleGCA(res_point* user, map<int, double> *seen_usersMAP, vector <pair<int, int > >* seen_usersSOC){
	double B = 0;
	int M = 2640;
	double dist_t  = (EARTH_CIRCUMFERENCE/360) * user->dist;
	//int size = seen_usersMAP->size();
	//cout << " |seen_usersMAP size| = " << size <<endl;
	//cout << " |seen_usersSOC size| = " << seen_usersSOC->size() <<endl;
	//double b = 1/(c_userDist+ 2*dist_t);
	double b = 0;
	double c_userDist = 0;

	int userID;
	priority_queue<double> qDouble ;
	list<pair<int, int> >* P = NULL;

	vector <pair<int, int > >::iterator itVEC = seen_usersSOC->begin();
	userID = (*itVEC).first;
	
	c_userDist = (seen_usersMAP->find(userID))->second;
	b = pow(EULERS_NUMBER,-(c_userDist + 2*dist_t ));
	

	cout<<"b = "<<b<<endl;

	
	map<int, list<pair<int, int> >* >::iterator it ;
	for(it = triangles.begin(); it != triangles.end() ; it++){
		int _1 = it->first;
		map<int, double >::iterator U = seen_usersMAP->find(_1);
		double _1dist = (U->second);
		P = it->second;
		//cout<<"userID = "<<_1<<" ==== num of triangles = "<<P->size()<<" ==== ";
		for (list<pair<int, int> >::iterator iterator = P->begin(); iterator != P->end(); ++iterator) {
			pair<int, int> fPair = *iterator;
			int firstFriend = fPair.first;
			int secondFriend = fPair.second;
			
			map<int, double >::iterator F1 = seen_usersMAP->find(firstFriend);
			map<int, double >::iterator F2 = seen_usersMAP->find(secondFriend);
			double dist_i = (F1->second) ;
			double dist_j = (F2->second) ;
			//cout<<endl <<"dist_i = "<<dist_i<<" | dist_j = "<<dist_j<<endl;
			//cout <<"_1dist = "<<_1dist<<" | dist_t = "<<dist_t<<endl;
			double tri_dist = pow(EULERS_NUMBER,-(dist_i + dist_j + _1dist ));
			
			//cout<<"Tri score dist_t = "<<tri<<" | using _1dist = "<<tri_dist<<endl;
			if(tri_dist > b) {
				//cout<<tri<<" ";
				qDouble.push(tri_dist);
			}
		}
	}

	int counter =0;
	if(user->id == 10489)
		cout<<"FUCK FUCK : "<<endl;
	for (int i =0; i < M ;i++){
		if(!qDouble.empty()){
			counter++;
			if(user->id == 10489)
				cout<<qDouble.top()<<endl;
			B+=qDouble.top();
			qDouble.pop();
		}
		else{
			B+= b;
		}
	}

	return B;
}
*/


double topkUsers::ScoreModuleGCA(res_point* user, map<int, double> *seen_usersMAP, vector <pair<int, int > >* seen_usersSOC, double beta){
	double B = 0;
	int M = 2640;
	double dist_t  = (EARTH_CIRCUMFERENCE/360) * user->dist;
	//int size = seen_usersMAP->size();
	//cout << " |seen_usersMAP size| = " << size <<endl;
	//cout << " |seen_usersSOC size| = " << seen_usersSOC->size() <<endl;
	//double b = 1/(c_userDist+ 2*dist_t);
	double b = 0;
	double c_userDist = 0;
	int userID;
	priority_queue<double> qDouble ;
	
	vector <pair<int, int > >::iterator itVEC = seen_usersSOC->begin();
	
	userID = (*itVEC).first;
	
	c_userDist = (seen_usersMAP->find(userID))->second;
	b = pow(EULERS_NUMBER,-(c_userDist + 2*dist_t )/beta);
	
	// cout<<"b = "<< b <<endl;
	
	int counter =0;
	double tri_score =0;
	multiset<double, greater<double> >::iterator multisetIter;
	for(multisetIter = set_of_scores.begin();multisetIter!=set_of_scores.end(); multisetIter++){
		if(counter < M){
			tri_score = *multisetIter;
			if(tri_score > b){
				B += tri_score;
				counter++;
			}
			else{
				break;
			}
		}
		else{
			break;
		}
	}

	if(counter<M){
		B+= b*(M-counter);
		counter+=(M-counter);
	}

	return B;
}	

struct GST_User
{
	const int id;
	double score;
	int counter;
	int Mvr;
	double Bvr;
	double bvr;
	
	GST_User(int _id, double _score, int _counter ) :
	id(_id), score(_score), counter(_counter) {Bvr=0;bvr=0;}
};

double topkUsers::print_time(struct timeval &start, struct timeval &end){
    double usec;
    usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    return usec;
}

//TODOs
//1. keep the top-k / unique pairs in topk? is it based on the value only?
//2. seen_users must be sorted //DONE using sorted set logn search time and logn insert.
//3. do not use arefriends
//4. groups may contain a friend many times - duplicates //removeDuplicates implemented in Group.cpp
//5. What if the result set is less than k?
//6. b=10 GCA completed printed whereas for b=100 not printed
priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkUsers::GCA(double x, double y,unsigned int k, int incrStep, double beta){

	struct timeval startExamine, endExamine, startAdd, endAdd;
	struct timeval startRefinement,endRefinement,startBound, endBound;
	double sumExamine =0, sumAdd =0, sumBound =0;
	
	usersChecked = 0;
	double DIST_LAST_USER;
	
	int M = 2640;
	double best_score = -1;
	double lb = 999999;
	res_point* newP;
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();

	multiset<my_pair, pair_comparator_descending>* topk = new multiset<my_pair, pair_comparator_descending>;	

	vector <pair<int, int > >* seen_usersSOC = new vector <pair<int, int > > ();
	
	set<int, int_comparator_descending>* seen_users = new set<int, int_comparator_descending>();
	map <int, double>* seen_usersMAP = new map<int, double>();
	
	map<int, GST_User*> GST_USER;
	
	map<int,Group*> groups;
	map<int,res_point*> points;
	int* friends;
	unsigned int friendsSize = 0 ;
	double W = -1;
	double old_score = -1;
	bool flag;
	double new_score;
	list<pair<int, int> >* P = NULL;
	usersChecked =0;
	threshold =0;
	
	newP = gpos->getNextNN(x, y, incrStep);
	//newP = gpos->getNextNN(x, y);
	seen_users->insert(newP->id);

	int examineP = 1; int addP = 1;
	
	while(lb > best_score && newP != NULL && topk->size() <= k){
		// cout << "-------------------------------------------------------------------------" << endl;
		// cout << "user = " << newP->id << "\t dist = " << newP->dist <<" | "<<(EARTH_CIRCUMFERENCE/360) * newP->dist<<endl;
		
		vector<int>* friendsSeen = new vector<int>();
		vector<int>* friendsUnseen = new vector<int>();

		//        cout << "---------------------- Top-" << k << endl;
		//        multiset<my_pair,pair_comparator_descending >::iterator topkIter;
		//        for(topkIter = topk->begin() ;topkIter!=topk->end();topkIter++){
		//            cout<<"User ID = "<<(*topkIter).getId()<<endl;
		//            cout<<"Score = "<<(*topkIter).getScore()<<endl;
		//        }
		//        cout << "----------------------" << endl;

		//        cout << "---------------------- scores " << endl;
		//        map<int,double>::iterator scoresIter;
		//        for(scoresIter = scores.begin() ;scoresIter!=scores.end();scoresIter++){
		//            cout<<"User ID = "<<(*scoresIter).first<< "\t Score = "<<(*scoresIter).second<<endl;
		//        }

		//        cout << "---------------------- triangles ----------------------------------------" << endl;
		//        map<int, list<pair<int, int> >* >::iterator trianIter;
		//        for(trianIter = triangles.begin() ;trianIter!=triangles.end();trianIter++){
		//            cout<<"User ID = "<<(*trianIter).first << "Troangles: ";
		//            P = trianIter->second;
		//            for (list<pair<int, int> >::iterator iterator = P->begin(); iterator != P->end(); ++iterator) {
		//                pair<int, int> fPair = *iterator;

		//                int firstFriend = fPair.first;
		//                int secondFriend = fPair.second;

		//                cout << " (" << firstFriend << ", " << secondFriend << ") ";

		//            }
		//            cout << endl;
		//        }

		//        cout << "-------------------------------------------------------------------------" << endl;

		W = -1;
		//cout << "i = " << i << "\t id = " << newP->id << endl;
		spos->getFriends(newP->id, friends, friendsSize);
		//numOfFriends.insert(make_pair(newP->id, friendsSize));
		int possibleTrianlges = (friendsSize * (friendsSize-1)/2 );
		seen_usersSOC->push_back(make_pair(newP->id, possibleTrianlges));
		
		GST_User* gstU = NULL;
		
		Group *nG = new Group(newP);
		points.insert(make_pair(newP->id, newP));

		map<int, list<pair<int, int> >* >::iterator it = triangles.find(newP->id);

		if(it != triangles.end()){
			//            cout << "--------------------------------- We found some triangles !!!!" << endl;
			//element found;
			gettimeofday(&startExamine, NULL);

			P = it->second;
			
			examineP = examineP + P->size();
			
			// list<pair<int, int> >* copyListPairs = new list<pair<int, int> >(P->begin(), P->end());
			// completeTriangles.insert(make_pair(newP->id, copyListPairs));
			//W = pow(EULERS_NUMBER,-((EARTH_CIRCUMFERENCE/360)*util.computeMinimumDistance(newP->x,newP->y,x,y)/beta));
			
			double dist_U  = (EARTH_CIRCUMFERENCE/360) * newP->dist;
			W = 0;
			
			//Group* newG = new Group(newP);
			
			for (list<pair<int, int> >::iterator iterator = P->begin(); iterator != P->end(); ++iterator) {
				pair<int, int> fPair = *iterator;

				int firstFriend = fPair.first;
				int secondFriend = fPair.second;

				//cout<<"size of P = "<<P->size()<<" F1 = "<<firstFriend<<" F2 = "<<secondFriend <<endl;
				
				res_point* res_F1 = (points.find(firstFriend))->second;
				res_point* res_F2 = (points.find(secondFriend))->second;
				
				double dist_F1 = (EARTH_CIRCUMFERENCE/360)*util.computeMinimumDistance(res_F1->x,res_F1->y,x,y);
				double dist_F2 = (EARTH_CIRCUMFERENCE/360)*util.computeMinimumDistance(res_F2->x,res_F2->y,x,y);
				
				double uvzScore = pow(EULERS_NUMBER,-(dist_F1 + dist_F2 + dist_U)/beta) ;

				W+=uvzScore;
				// ---------------------------------------- first user ---------------------------------

				gstU = (GST_USER.find(firstFriend))->second;
				old_score = gstU->score;
				new_score = old_score + (uvzScore);
				gstU->score = new_score;
				gstU->counter = gstU->counter + 1;

				
				res_point* res_friend = (points.find(firstFriend))->second;
				nG->addFriend(res_friend);

				Group* firstFriendGroup = (*(groups.find(firstFriend))).second;
				firstFriendGroup->addFriend(newP);
				firstFriendGroup->addFriend((*(points.find(secondFriend))).second);


				if(new_score > best_score){
					// let pos be the position using find
					flag = false;
					multiset<my_pair,pair_comparator_descending >::const_iterator pos = find_if(topk->begin(), topk->end(), MatchPairSecond(old_score)); //use find(newP->id, score);

					while(pos != topk->end() && !flag && (*pos).getScore() == old_score){
						if((*pos).getId() == firstFriend)
						flag = true;
						else
						pos++;
					}

					if(flag){// he is in topk
						topk->erase(pos);
					}
					my_pair user = my_pair(firstFriend,new_score);
					topk->insert(user);

					if(topk->size() > k){
						topk->erase(--(topk->rbegin()).base());
						best_score = (*topk->rbegin()).getScore();
					}
					else if(topk->size() == k){
						best_score = (*topk->rbegin()).getScore();
					}
				}

				//-----------------------------  second user --------------------------------------------

				gstU = (GST_USER.find(secondFriend))->second;
				old_score = gstU->score;
				new_score = old_score+(uvzScore);

				gstU->score = new_score;
				gstU->counter = gstU->counter + 1;
				
				res_friend = (points.find(secondFriend))->second;
				nG->addFriend(res_friend);

				Group* secondFriendGroup = (*groups.find(secondFriend)).second;
				secondFriendGroup->addFriend(newP);
				secondFriendGroup->addFriend((*(points.find(firstFriend))).second);

				if(new_score > best_score){
					// let pos be the position using find
					flag = false;
					multiset<my_pair,pair_comparator_descending >::const_iterator pos = find_if(topk->begin(), topk->end(), MatchPairSecond(old_score)); //use find(newP->id, score);

					while(pos != topk->end() && !flag && (*pos).getScore() == old_score){
						if((*pos).getId() == secondFriend)
						flag = true;
						else
						pos++;
					}

					if(flag){// he is in topk
						topk->erase(pos);
					}
					my_pair user = my_pair(secondFriend,new_score);
					topk->insert(user);

					if(topk->size() > k){
						topk->erase(--(topk->rbegin()).base());
						best_score = (*topk->rbegin()).getScore();
					}
					else if(topk->size() == k){
						best_score = (*topk->rbegin()).getScore();
					}
				}

			}
			groups.insert(make_pair(newP->id, nG));
			
			
			
			if(friendsSize>=2)
			new_score = W;
			else
			new_score = 0;
			//newG->score = score;
			my_pair user = my_pair(newP->id,new_score);
			//scores.insert(make_pair(newP->id,new_score));
			GST_User* g_user = new GST_User(newP->id,new_score,P->size());
			GST_USER.insert(make_pair(newP->id,g_user));
			
			
			// if his value is greater than the best_score, then do the following
			// find the position of the user in topk set
			// if he is not in topk then simple add him
			// if he is in topk then delete him and add him with the new score
			// if there are more than k users in topk, then keep only the k first users
			// update best_score with the score of the kth user

			if(new_score > best_score){
				topk->insert(user);
				if(topk->size() > k){
					topk->erase(--(topk->rbegin()).base());
					best_score = (*topk->rbegin()).getScore();
				}
				else if(topk->size() == k){
					best_score = (*topk->rbegin()).getScore();
				}
			}
			
			triangles.erase(it);
			// cout<<"erased from triangles id = "<<newP->id<<" | #triangles erased ="<<P->size()<<endl;
			
			//erase from map of iter and perform deep clean from set_of_scores
			map < int, list< multiset<double, greater<double> >::iterator >* >::iterator itermap = map_of_iter.find(newP->id);
			list< multiset<double, greater<double> >::iterator >* list_of_iter;			
			list_of_iter = itermap->second;
			
			list< multiset<double, greater<double> >::iterator >::iterator iter_of_list;
			for(iter_of_list = list_of_iter->begin() ; iter_of_list!=list_of_iter->end() ; iter_of_list++)
				set_of_scores.erase(*iter_of_list);
				
			// cout<<"erased from map_of_iter id = "<<newP->id<<" | #list members erased ="<<list_of_iter->size()<<endl;
			list_of_iter->clear();
			map_of_iter.erase(itermap);

			gettimeofday(&endExamine, NULL);
			sumExamine += print_time(startExamine, endExamine); 
			
		}
		else{
			groups.insert(make_pair(newP->id, new Group(newP)));
			
			new_score = 0;
			
			// find if already exists, update
			// else make new and insert
			my_pair user = my_pair(newP->id,new_score);
			GST_User* g_user = new GST_User(newP->id,new_score,0);
			GST_USER.insert(make_pair(newP->id,g_user));

			if(new_score > best_score){
				topk->insert(user);
				if(topk->size() > k){
					topk->erase(--(topk->rbegin()).base());
					best_score = (*topk->rbegin()).getScore();
				}
				else if(topk->size() == k){
					best_score = (*topk->rbegin()).getScore();
				}
			}
		}

		gettimeofday(&startAdd, NULL);
		
		for(unsigned int j=0; j< friendsSize; j++){
			friendsUnseen->push_back(friends[j]);
		}

		for(unsigned int j=0; j< friendsSize; j++){
			if(seen_users->find(friends[j])!=seen_users->end()){
				friendsSeen->push_back(friends[j]);
				friendsUnseen->erase(find(friendsUnseen->begin(), friendsUnseen->end(), friends[j]));
			}
		}
		
		vector<int>::iterator i;
		int trilistcounter =0, iterlistcounter=0;
		for(i = friendsUnseen->begin(); i != friendsUnseen->end() ; i++){
		
			spos->getFriends(*i, friends, friendsSize);
			//F_UNSEEN - > 3 4 5 6 (*i=3)
			vector<int>::iterator j;
			map<int, list<pair<int, int> >*>::iterator iter = triangles.find(*i);
			map < int, list< multiset<double, greater<double> >::iterator >* >::iterator itermap = map_of_iter.find(*i);
			
			list<pair<int, int> >* listTriangles;
			if (iter == triangles.end()) {
				listTriangles = new list<pair<int, int> >();
			}
			else{
				listTriangles = iter->second;
			}
			
			list< multiset<double, greater<double> >::iterator >* list_of_iter;			
			if(itermap == map_of_iter.end()){
				list_of_iter = new list< multiset<double, greater<double> >::iterator > ();
			}
			else{
				list_of_iter = itermap->second;
			}
			
			
			for(j = friendsSeen->begin(); j != friendsSeen->end() ; j++ ){
				
				//F_SEEN - > 1 2 (*j=1)
				if(util.binarySearch(friends, 0, friendsSize-1, *j)){
				
					map<int, double >::iterator mapDistIter;
					double dist_unseen = 0;
					mapDistIter = seen_usersMAP->find(*j);
					double dist_seen = (mapDistIter->second);
					
					listTriangles->push_back(make_pair(newP->id,*j));
					
					addP++;
					mapDistIter = seen_usersMAP->find(*i);
					if(mapDistIter == seen_usersMAP->end()){
						double userLocation[2];
						gpos->getLocation(*i, userLocation);
						if(userLocation[0]!=-1000){ // is not null
							dist_unseen = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
							res_point* rp = util.createResultPoint(*i, userLocation[0], userLocation[1], dist_unseen);
							seen_usersVEC->push_back(rp);
							points.insert(make_pair(*i, rp));
							dist_unseen = (EARTH_CIRCUMFERENCE/360)*dist_unseen;
							seen_usersMAP->insert(make_pair(*i,dist_unseen));
						}
					}
					else{
						mapDistIter = seen_usersMAP->find(*i);
						dist_unseen = (mapDistIter->second);
					}
					
					//cout<<"\t dist_unseen ("<< *i <<") = "<< dist_unseen << " | dist_seen (" << *j <<") = "<< dist_seen << " | newP->dist ("<<newP->id <<") = "<<(EARTH_CIRCUMFERENCE/360) * newP->dist<<endl;
					double tri_score = pow(EULERS_NUMBER,-(dist_unseen + dist_seen + (EARTH_CIRCUMFERENCE/360) * newP->dist)/beta);
					list_of_iter->push_back(set_of_scores.insert(tri_score));
				}
			}
			if (iter == triangles.end() && listTriangles->size()!=0) {
				triangles.insert(make_pair(*i, listTriangles));
				trilistcounter++;
			}
			if(itermap == map_of_iter.end() && list_of_iter->size()!=0){
				map_of_iter.insert(make_pair(*i, list_of_iter));
				iterlistcounter++;
			}
			
		}
		//cout<<"#users inserted into triangles = "<<trilistcounter<<endl;
		//cout<<"#users inserted into map_of_iter = "<<iterlistcounter<<endl;
		
		gettimeofday(&endAdd, NULL);
		sumAdd += print_time(startAdd, endAdd); // compute microsec

		delete friends;
		friendsSize = 0;
		friendsSeen->clear();
		delete friendsSeen;
		friendsUnseen->clear();
		delete friendsUnseen;

		if(newP != NULL){
			//seen_users->push_back(newP->id);
			seen_users->insert(newP->id);
			double userLocation[2];
			gpos->getLocation(newP->id, userLocation);
			if(userLocation[0]!=-1000){ // is not null
				double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				res_point* rp = util.createResultPoint(newP->id, userLocation[0], userLocation[1], dist);
				//candidatePoints->insert(rp);
				seen_usersVEC->push_back(rp);
				seen_usersMAP->insert(make_pair(newP->id,(EARTH_CIRCUMFERENCE/360)*dist));
			}
		}

		gettimeofday(&startBound, NULL);
		lb = ScoreModuleGCA(newP, seen_usersMAP,seen_usersSOC,beta);
		gettimeofday(&endBound, NULL);
		sumBound += print_time(startBound, endBound); 
		// cout << "best_score = " << best_score << "\t T = " << lb << endl;
		//cout << "Size of triangles = "<<triangles.size()<<endl;
		//cout << "Size of map of iter = "<<map_of_iter.size()<<endl;

		if(lb <= best_score){
			DIST_LAST_USER = newP->dist*(EARTH_CIRCUMFERENCE/360);
			cout <<"--------------------------------------" << endl;
			cout<< " lb = "<<lb<<endl;
			cout<<"GCA completed!" << endl;
			cout<<"Radius of search = "<<newP->dist <<" | "<<newP->dist*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
			cout<<"User Id of terminating user = "<<newP->id<<endl;
			cout <<"--------------------------------------" << endl;
			threshold = newP->dist;
			break;
		}
		newP = gpos->getNextNN(x, y, incrStep);
		//newP = gpos->getNextNN(x, y);
		//        cout <<"--------------------------------------" << endl;
		usersChecked++;
	}
	// cout<<"examine P counter ="<<examineP <<" | time taken to examine = "<<sumExamine/1000<<" ms"<<endl;
	// cout<<"add to P counter ="<<addP <<" | time taken to Add = "<<sumAdd/1000<<" ms"<<endl;

	// cout<<"Time for computing bound = "<<sumBound/1000<<" ms"<<endl;
	multiset<my_pair,pair_comparator_descending >::iterator topkIter;
	
	//--------------------------------------------------
	//-------------Refinement Stage---------------------
	//--------------------------------------------------
	
	gettimeofday(&startRefinement, NULL);
	
	multiset<my_pair, pair_comparator_descending>* topkComplete = new multiset<my_pair, pair_comparator_descending>;
	unsigned int FfriendsSize=0;
	int* Ffriends;
	
	// cout<<"OLD BEST SCORE ="<<best_score<<endl;
	double dist_U =0,dist_F=0,dist_FF=0;

	for(topkIter = topk->begin() ;topkIter!=topk->end();topkIter++){
		new_score = 0;
		int topkID = (*topkIter).getId();
		spos->getFriends(topkID, friends, friendsSize);
		map<int, double >::iterator iter_U = seen_usersMAP->find(topkID);
		dist_U 	= (iter_U->second);
		// user top-k - > 1 2 3 4 5 6
		
		Group* topkGP = (*groups.find(topkID)).second;
		
		for(unsigned int i = 0; i< friendsSize; i++ ){
			
			spos->getFriends(friends[i], Ffriends, FfriendsSize);
			// user 1 - > 2 4 9 10 11 12
			// user 4 - > 1 9 16 21
			for(unsigned int j = 0; j< FfriendsSize; j++ ){
				
				if(util.binarySearch(friends, i+1, friendsSize-1, Ffriends[j])){
					map<int, double >::iterator iter_F = seen_usersMAP->find(friends[i]);
					res_point* rpF;
					res_point* rpFF;

					if(iter_F != seen_usersMAP->end()){
						dist_F = (iter_F->second);
						rpF = (points.find(friends[i]))->second;
					}
					else{
						double userLocation[2];
						gpos->getLocation(friends[i], userLocation);
						if(userLocation[0]!=-1000){ // is not null
							dist_F =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
							rpF = util.createResultPoint(friends[i], userLocation[0], userLocation[1], dist_F);
							seen_usersMAP->insert(make_pair(friends[i],(EARTH_CIRCUMFERENCE/360)*dist_F));
							points.insert(make_pair(friends[i],rpF));
							dist_F = (EARTH_CIRCUMFERENCE/360)*dist_F;
						}
						else
						cout<<"ERROR"<<endl;
					}

					map<int, double >::iterator iter_FF = seen_usersMAP->find(Ffriends[j]);
					
					if(iter_FF != seen_usersMAP->end()){
						dist_FF = (iter_FF->second) ;
						rpFF = (points.find(Ffriends[j]))->second;
					}
					else{
						double userLocation[2];
						gpos->getLocation(Ffriends[j], userLocation);
						if(userLocation[0]!=-1000){ // is not null
							dist_FF =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
							rpFF = util.createResultPoint(Ffriends[j], userLocation[0], userLocation[1], dist_FF);
							seen_usersMAP->insert(make_pair(Ffriends[j],(EARTH_CIRCUMFERENCE/360)*dist_FF));
							points.insert(make_pair(Ffriends[j],rpFF));
							dist_FF = (EARTH_CIRCUMFERENCE/360)*dist_FF;
						}
						else
						cout<<"ERROR"<<endl;
					}

					topkGP->addFriend(rpF);
					topkGP->addFriend(rpFF);

					//cout<<"dist_U ("<<topkID<<") = "<<dist_U<<" | dist_F ("<<friends[i]<<") = "<<dist_F<<" | dist_FF ("<<Ffriends[j]<<") = "<<dist_FF<<endl;
					//cout<<"triangle score = "<<pow(EULERS_NUMBER,-(dist_F + dist_FF + dist_U))<<endl;
					new_score += pow(EULERS_NUMBER,-(dist_F + dist_FF + dist_U)/beta) ;

				}
			}
			
			delete Ffriends;
			FfriendsSize=0;
		}
		delete friends;
		friendsSize =0;
		my_pair user = my_pair(topkID,new_score);
		topkComplete->insert(user);
	}
	best_score = (*topkComplete->rbegin()).getScore();
	// cout <<"BEST SCORE IS NOW = "<<best_score<<endl;
	
	///*

	map<int, GST_User*>::iterator user_IT = GST_USER.begin();
	// cout<<"SIZE OF GST_USER = "<<GST_USER.size()<<endl;
	while(user_IT != GST_USER.end()){
		GST_User* vr = user_IT->second;	
		int vrID = user_IT->first;
		// cout<<"ID = "<<vrID<<" | score = "<<vr->score<<" | OLD COUNTER ="<<vr->counter;
		if(vr->score == 0){
			delete vr;
			vr = NULL;
			GST_USER.erase(user_IT++);
			// cout<<" DELETED!"<<endl;
			continue;
		}
		else{
			bool isDel=false;
			for(topkIter = topk->begin() ;topkIter!=topk->end();topkIter++){
				if(vrID==(*topkIter).getId()){
					delete vr;
					vr = NULL;
					GST_USER.erase(user_IT++);
					isDel=true;
					// cout<<" DELETED!"<<endl;
					break;
				}
			}
			if(isDel==true)
				continue;
		}
		
		double dist_vr = (seen_usersMAP->find(vrID))->second;	
		int deg_vr =spos->getUserDegree(vrID);
		int numOfTriangles = deg_vr*(deg_vr-1)/2;

		vr->Mvr = min(M, numOfTriangles);
		vr->bvr = pow(EULERS_NUMBER,-(dist_vr + 2*DIST_LAST_USER )/beta);
		++user_IT;	
		// cout<<" | bvr = "<<vr->bvr<<endl;
	}
	// cout<<"SIZE OF GST_USER after pruning = "<<GST_USER.size()<<endl;
	
	
	map<int, list<pair<int, int> >* >::iterator it ;
	for(it = triangles.begin(); it != triangles.end() ; it++){
		int _1 = it->first;
		map<int, double >::iterator U = seen_usersMAP->find(_1);
		double _1dist = (U->second);
		P = it->second;
		//cout<<"userID = "<<_1<<" ==== num of triangles = "<<P->size()<<" ==== ";
		for (list<pair<int, int> >::iterator iterator = P->begin(); iterator != P->end(); ++iterator) {
			pair<int, int> fPair = *iterator;
			int firstFriend = fPair.first;
			int secondFriend = fPair.second;
			
			map<int, double >::iterator F1_distIT = seen_usersMAP->find(firstFriend);
			map<int, double >::iterator F2_distIT = seen_usersMAP->find(secondFriend);
			double dist_i = (F1_distIT->second) ;
			double dist_j = (F2_distIT->second) ;
			
			//cout<<"dist_i ("<<firstFriend<<") = "<<dist_i<<" | dist_j ("<<secondFriend<<") = "<<dist_j<<" | _1dist ("<<_1<<") = "<<_1dist<<endl;
			double tri_scr = pow(EULERS_NUMBER,-(dist_i + dist_j + _1dist )/beta);
			
			map<int, GST_User*>::iterator F1_ITER = GST_USER.find(firstFriend);
			map<int, GST_User*>::iterator F2_ITER = GST_USER.find(secondFriend);
			
			if(F1_ITER!=GST_USER.end()){
			GST_User* F1 = F1_ITER->second;
				if(tri_scr > F1->bvr) {			
					F1->score = F1->score + tri_scr;
					F1->counter = F1->counter + 1;
				}
			}
			
			if(F2_ITER!=GST_USER.end()) {
			GST_User* F2 = F2_ITER->second;
				if(tri_scr > F2->bvr) {			
					F2->score = F2->score + tri_scr;
					F2->counter = F2->counter + 1;
				}
			}
		}
	}


	for(user_IT = GST_USER.begin(); user_IT != GST_USER.end(); user_IT++){
		GST_User* vr = user_IT->second;	
		
		int vrID = user_IT->first;
		
		if(vr->counter <= vr->Mvr)
			vr->Bvr = vr->score + (vr->Mvr -  vr->counter)*vr->bvr;
			
		if(vr->Bvr > best_score){
			new_score = 0;
			spos->getFriends(vrID, friends, friendsSize);
			map<int, double >::iterator iter_U = seen_usersMAP->find(vrID);
			dist_U 	= (iter_U->second);
			// user vr - > 1 2 3 4 5 6
			
			Group* vrGP = (*groups.find(vrID)).second;
			//int ctr=0;int inCtr=0;int completeTri=0;int completeTribefore=0;
			for(unsigned int i = 0; i< friendsSize; i++ ){
				
				spos->getFriends(friends[i], Ffriends, FfriendsSize);
				// user 1 - > 2 4 9 10 11 12
				// user 4 - > 1 9 16 21
					for(unsigned int j = 0; j< FfriendsSize; j++ ){
					
						if(util.binarySearch(friends, i+1, friendsSize-1, Ffriends[j])){
							map<int, double >::iterator iter_F = seen_usersMAP->find(friends[i]);
							res_point* rpF;
							res_point* rpFF;

							if(iter_F != seen_usersMAP->end()){
								dist_F = (iter_F->second);
								rpF = (points.find(friends[i]))->second;
							}
							else{
								double userLocation[2];
								gpos->getLocation(friends[i], userLocation);
								if(userLocation[0]!=-1000){ // is not null
									dist_F =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
									rpF = util.createResultPoint(friends[i], userLocation[0], userLocation[1], dist_F);
									seen_usersMAP->insert(make_pair(friends[i],(EARTH_CIRCUMFERENCE/360)*dist_F));
									points.insert(make_pair(friends[i],rpF));
									dist_F = (EARTH_CIRCUMFERENCE/360)*dist_F;
								}
								else
									cout<<"ERROR"<<endl;
							}

							map<int, double >::iterator iter_FF = seen_usersMAP->find(Ffriends[j]);
							
							if(iter_FF != seen_usersMAP->end()){
								dist_FF = (iter_FF->second) ;
								rpFF = (points.find(Ffriends[j]))->second;
							}
							else{
								double userLocation[2];
								gpos->getLocation(Ffriends[j], userLocation);
								if(userLocation[0]!=-1000){ // is not null
									dist_FF =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
									rpFF = util.createResultPoint(Ffriends[j], userLocation[0], userLocation[1], dist_FF);
									seen_usersMAP->insert(make_pair(Ffriends[j],(EARTH_CIRCUMFERENCE/360)*dist_FF));
									points.insert(make_pair(Ffriends[j],rpFF));
									dist_FF = (EARTH_CIRCUMFERENCE/360)*dist_FF;
								}
								else
									cout<<"ERROR"<<endl;
							}

							vrGP->addFriend(rpF);
							vrGP->addFriend(rpFF);
							
							new_score += pow(EULERS_NUMBER,-(dist_F + dist_FF + dist_U)/beta) ;

							// if(vrID == 2000){
								// cout<<"\t dist_U ("<<vrID<<") = "<<dist_U<<" | dist_F ("<<friends[i]<<") = "<<dist_F<<" | dist_FF ("<<Ffriends[j]<<") = "<<dist_FF<<endl;
								// double tscr = pow(EULERS_NUMBER,-(dist_F + dist_FF + dist_U));
								// cout<<"\t triangle score = "<<tscr<<" | cumulative score="<<new_score<<endl;
								// if(tscr > vr->bvr)
									// cout<<"\t # triangles where Tscr>bvr  ="<<inCtr++<<" | DIST_LAST_USER= "<<DIST_LAST_USER<<endl; 
								// if(dist_F < DIST_LAST_USER && dist_FF < DIST_LAST_USER)
									// cout<<"\t # complete triangles  ="<<completeTri++<<endl; 
								// if(dist_F < dist_U && dist_FF < dist_U)
									// cout<<"\t # complete triangles before  ="<<completeTribefore++<<endl; 
								// cout<<"------------------------Tri : "<<ctr++<<"--------------------------------"<<endl;
							// }
							
						}
					}
					
				delete Ffriends;
				FfriendsSize=0;
			}
			
			delete friends;
			friendsSize =0;
			my_pair user = my_pair(vrID,new_score);

			if(new_score > best_score){
				topkComplete->insert(user);
				if(topkComplete->size() > k){
					topkComplete->erase(--(topkComplete->rbegin()).base());
					best_score = (*topkComplete->rbegin()).getScore();
				}
				else if(topkComplete->size() == k){
					best_score = (*topkComplete->rbegin()).getScore();
				}
			}
		}
	}

	gettimeofday(&endRefinement, NULL);
	// cout<<"time taken for refinement = "<<print_time(startRefinement, endRefinement)/1000<<" ms"<<endl;
	
	//-------------------------------------------------
	//---------------end of Refinement-----------------
	//-------------------------------------------------
	
	for(topkIter = topkComplete->begin() ;topkIter!=topkComplete->end();topkIter++){

		int userID = (*topkIter).getId();
		Group* GP = (*(groups.find(userID))).second;
		GP->score = (*topkIter).getScore();
		heap->push(new Group(GP));
	}
	
	seen_users->clear();
	GST_USER.clear();
	triangles.clear();
	
	set_of_scores.clear();
	map_of_iter.clear();
	
	seen_usersVEC->clear();
	gpos->clearNextNN();
	delete seen_users;
	return heap;
}


double topkUsers::GST_Lazy_Bound(res_point* user, vector<double>* M_bestVEC, double beta){
	double T = 0;
	double dist_U  = (EARTH_CIRCUMFERENCE/360) * user->dist;
	vector<double>::iterator iter; 
	for(iter = M_bestVEC->begin(); iter!= M_bestVEC->end() ;iter++){
			T +=  pow(EULERS_NUMBER,-((*iter + dist_U))/beta) ;
	}

	return T;
}	

////////////////////////////////////////
////////////////////////////////////////


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkUsers::GST_Lazy(double x, double y,unsigned int k, int incrStep, double beta){

	// struct timeval startRefinement,endRefinement,startBound,endBound, startMain, endMain, startMT, endMT, startA,endA,startB,endB,startC,endC,startNN,endNN;
	// double sumBound =0, sumA=0,sumB=0,sumC=0;

	
	double DIST_LAST_USER;
	
	unsigned int M = 2640;
	double best_score = -1;
	double lb = 999999;
	usersChecked =0;
	threshold =0;	
	
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();

	multiset<my_pair, pair_comparator_descending>* topk = new multiset<my_pair, pair_comparator_descending>;	
	
	map<int, GST_User*> GST_USER;
	
	map<int,Group*> groups;
	map<int,res_point*> points;
	map<int,int> maxTriangles;
	
	
	int* friends;
	unsigned int friendsSize = 0 ;
	int* Ffriends;
	unsigned int FfriendsSize = 0 ;


	// calculate M best triangles
	priority_queue<double, vector<double>, greater<double> >* M_bestQ = new priority_queue<double, vector<double>, greater<double> >() ;
	vector<res_point*>* MNN = gpos->getkNN(x, y, M);
	
	
	// gettimeofday(&startMT, NULL);
	double best_dist = 99999;
	for (unsigned int i = 0; i < MNN->size()-1; i++){
		res_point* U1 = MNN->at(i);
		
		for(unsigned int j = i + 1; j < MNN->size(); j++){
			res_point* U2 = MNN->at(j);
			
			double localdist = (U1->dist+U2->dist)*(EARTH_CIRCUMFERENCE/360);
			if(localdist < best_dist){
				M_bestQ->push(localdist);
				
				if(M_bestQ->size() > M){
					M_bestQ->pop();
					best_dist =	M_bestQ->top();
				}
				else if(M_bestQ->size() == M){
					best_dist = M_bestQ->top();
				}	
			}
			else
				break;
		}
	}
	
	vector<double>* M_bestVEC = new vector<double>();
	
	while(!M_bestQ->empty()) {
		M_bestVEC->push_back(M_bestQ->top());
		M_bestQ->pop();
	}
	//--------------------------------
	// gettimeofday(&endMT, NULL);
	//cout<<"M_bestVEC->size() = " <<M_bestVEC->size() <<"| Time for computing = "<<print_time(startMT, endMT)/1000<<" ms"<<endl;
    // cout<<"Time for computing M_Best = "<<print_time(startMT, endMT)/1000<<" ms"<<endl;
	
	res_point* newP;
	
	// gettimeofday(&startNN, NULL);
	newP = gpos->getNextNN(x, y, incrStep);
	// newP = gpos->getNextNN(x, y);
	// gettimeofday(&endNN, NULL);
	// cout<<"Total Time for first getNearestUser= "<<print_time(startNN, endNN)/1000<<" ms"<<endl;

	// gettimeofday(&startMain, NULL);
	

	while(lb > best_score && newP != NULL && topk->size() <= k){
		//cout << "-------------------------------------------------------------------------" << endl;
		//cout << "user = " << newP->id << "\t dist = " << newP->dist <<" | "<<(EARTH_CIRCUMFERENCE/360) * newP->dist<<endl;
		
		// gettimeofday(&startA, NULL);
		
		points.insert(make_pair(newP->id, newP));
		groups.insert(make_pair(newP->id, new Group(newP)));
		
		spos->getFriends(newP->id, friends, friendsSize);
	
		int possibleTrianlges = (friendsSize * (friendsSize-1)/2 );
		maxTriangles.insert(make_pair(newP->id, possibleTrianlges));
		
		vector<res_point*>* friendsVEC = new vector<res_point*>() ;	
		//identify friends in range
		for (unsigned int m = 0; m < friendsSize; m++){
			
			map<int,res_point*>::iterator localMapIT = points.find(friends[m]);
			if(localMapIT != points.end()) {
				res_point* rp = localMapIT->second;
				friendsVEC->push_back(rp);
			}
		}
		
		GST_User* gstF = NULL;
		GST_User* gstFF = NULL;
		
		// gettimeofday(&endA, NULL);
		// sumA+=print_time(startA, endA);
		
		// gettimeofday(&startC, NULL);
		//identify all triangles among the friends
		double Uscore = 0; double Ucounter = 0; bool flag = false;
		double dist_U = 0, dist_F = 0, dist_FF = 0 ;
		dist_U = newP->dist;
		for (unsigned int i = 0; i < friendsVEC->size(); i++){
			res_point* rpF = friendsVEC->at(i);
			spos->getFriends(rpF->id, Ffriends, FfriendsSize);	
			dist_F = rpF->dist;
			gstF = (GST_USER.find(rpF->id))->second;
			double Fold_scr = gstF->score;
			
			for(unsigned int j = i + 1; j < friendsVEC->size(); j++){
				res_point* rpFF = friendsVEC->at(j);
				
				if(util.binarySearch(Ffriends, 0, FfriendsSize-1, rpFF->id)){
					
					gstFF = (GST_USER.find(rpFF->id))->second;
					dist_FF = rpFF->dist;
					double FFold_scr = gstFF->score;
					
					//cout<<"dist_U ("<<topkID<<") = "<<dist_U<<" | dist_F ("<<friends[i]<<") = "<<dist_F<<" | dist_FF ("<<Ffriends[j]<<") = "<<dist_FF<<endl;
					double tri_scr = pow(EULERS_NUMBER,-((dist_F + dist_FF + dist_U)*(EARTH_CIRCUMFERENCE/360))/beta) ;
					//cout<<"triangle score = "<<tri_scr<<endl;
					
					Uscore = Uscore + tri_scr;
					Ucounter = Ucounter + 1;
					gstF->score = gstF->score + tri_scr;
					gstF->counter = gstF->counter + 1;
					gstFF->score = gstFF->score + tri_scr;
					gstFF->counter = gstFF->counter + 1;
					
					if(gstFF->score > best_score){
						flag = false;
						multiset<my_pair,pair_comparator_descending >::const_iterator pos = find_if(topk->begin(), topk->end(), MatchPairSecond(FFold_scr));

						while(pos != topk->end() && !flag && (*pos).getScore() == FFold_scr){
							if((*pos).getId() == rpFF->id)
							flag = true;
							else
							pos++;
						}

						if(flag){// he is in topk
							topk->erase(pos);
						}
						my_pair user = my_pair(rpFF->id,gstFF->score);
						topk->insert(user);

						if(topk->size() > k){
							topk->erase(--(topk->rbegin()).base());
							best_score = (*topk->rbegin()).getScore();
						}
						else if(topk->size() == k){
							best_score = (*topk->rbegin()).getScore();
						}
					}
				
				}
			}
			delete Ffriends;
			FfriendsSize=0;
			
			//update F in R
			if(gstF->score > best_score){
				flag = false;
				multiset<my_pair,pair_comparator_descending >::const_iterator pos = find_if(topk->begin(), topk->end(), MatchPairSecond(Fold_scr));

				while(pos != topk->end() && !flag && (*pos).getScore() == Fold_scr){
					if((*pos).getId() == rpF->id)
					flag = true;
					else
					pos++;
				}

				if(flag){// he is in topk
					topk->erase(pos);
				}
				my_pair user = my_pair(rpF->id,gstF->score);
				topk->insert(user);

				if(topk->size() > k){
					topk->erase(--(topk->rbegin()).base());
					best_score = (*topk->rbegin()).getScore();
				}
				else if(topk->size() == k){
					best_score = (*topk->rbegin()).getScore();
				}
			}
			
		}
		
		// gettimeofday(&endC, NULL);
		// sumC+=print_time(startC, endC);
		
		my_pair user = my_pair(newP->id,Uscore);
		GST_User* gstU = new GST_User(newP->id,Uscore,Ucounter);
		GST_USER.insert(make_pair(newP->id,gstU));
		

		if(Uscore > best_score){
			topk->insert(user);
			if(topk->size() > k){
				topk->erase(--(topk->rbegin()).base());
				best_score = (*topk->rbegin()).getScore();
			}
			else if(topk->size() == k){
				best_score = (*topk->rbegin()).getScore();
			}
		}
		
		// gettimeofday(&startBound, NULL);
		lb = GST_Lazy_Bound(newP, M_bestVEC, beta);
		// gettimeofday(&endBound, NULL);
		// sumBound+=print_time(startBound, endBound);
		//cout << "best_score = " << best_score << "\t T = " << lb << endl;

		delete friends;
		friendsSize=0;

		if(lb <= best_score){
			DIST_LAST_USER = newP->dist;
			// cout <<"--------------------------------------" << endl;
			// cout<< "T = "<<lb<<endl;
			// cout<<"GST_lazy completed!" << endl;
			// cout<<"Radius of search = "<<newP->dist <<" | "<<newP->dist*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
			// cout<<"User Id of terminating user = "<<newP->id<<endl;
			// cout <<"--------------------------------------" << endl;
			threshold = newP->dist;
			break;
		}
		// gettimeofday(&startB, NULL);	
		newP = gpos->getNextNN(x, y, incrStep);
		//newP = gpos->getNextNN(x, y);
		// gettimeofday(&endB, NULL);
		// sumB+=print_time(startB, endB);
		
		usersChecked++;
		//        cout <<"--------------------------------------" << endl;
	}
	
	// gettimeofday(&endMain, NULL);
	// cout<<"Total Time for computing friendsInrange = "<<sumA/1000<<" ms"<<endl;
	// cout<<"Total Time for identifying all triangles among these friends = "<<sumC/1000<<" ms"<<endl;
	// cout<<"Total Time for computing getNextNN in stage 1 = "<<sumB/1000<<" ms"<<endl;
	// cout<<"Total Time for computing bound in stage 1 = "<<sumBound/1000<<" ms"<<endl;
	// cout<<"Total Time for stage 1 = "<<print_time(startMain, endMain)/1000<<" ms"<<endl;
	
	multiset<my_pair,pair_comparator_descending >::iterator topkIter;
	
	//--------------------------------------------------
	//-------------Refinement Stage---------------------
	//--------------------------------------------------
	
	// gettimeofday(&startRefinement, NULL);
	
	multiset<my_pair, pair_comparator_descending>* topkComplete = new multiset<my_pair, pair_comparator_descending>;
	
	//cout<<"OLD BEST SCORE ="<<best_score<<endl;
	
	for(topkIter = topk->begin() ;topkIter!=topk->end();topkIter++){
		double new_score = 0;
		int topkID = (*topkIter).getId();
		spos->getFriends(topkID, friends, friendsSize);
		map<int,res_point*>::iterator iter_U = points.find(topkID);
		res_point* rpU = (iter_U->second);
		// user top-k - > 1 2 3 4 5 6
		
		Group* topkGP = (*groups.find(topkID)).second;
		
		for(unsigned int i = 0; i< friendsSize; i++ ){
			
			spos->getFriends(friends[i], Ffriends, FfriendsSize);
			// user 1 - > 2 4 9 10 11 12
			// user 4 - > 1 9 16 21
			for(unsigned int j = 0; j< FfriendsSize; j++ ){
				
				if(util.binarySearch(friends, i+1, friendsSize-1, Ffriends[j])){
					map<int,res_point*>::iterator iter_F = points.find(friends[i]);
					res_point* rpF; double dist_F = 0;
					res_point* rpFF; double dist_FF = 0;

					if(iter_F != points.end()){
						rpF = iter_F->second;
					}
					else{
						double userLocation[2];
						gpos->getLocation(friends[i], userLocation);
						if(userLocation[0]!=-1000){ // is not null
							dist_F =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
							rpF = util.createResultPoint(friends[i], userLocation[0], userLocation[1], dist_F);
						}
						else
						cout<<"ERROR"<<endl;
					}

					map<int,res_point*>::iterator iter_FF = points.find(Ffriends[j]);
					
					if(iter_FF != points.end()){
						rpFF = iter_FF->second;
					}
					else{
						double userLocation[2];
						gpos->getLocation(Ffriends[j], userLocation);
						if(userLocation[0]!=-1000){ // is not null
							dist_FF =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
							rpFF = util.createResultPoint(Ffriends[j], userLocation[0], userLocation[1], dist_FF);
							points.insert(make_pair(Ffriends[j],rpFF));
						}
						else
						cout<<"ERROR"<<endl;
					}

					topkGP->addFriend(rpF);
					topkGP->addFriend(rpFF);

					//cout<<"dist_U ("<<topkID<<") = "<<dist_U<<" | dist_F ("<<friends[i]<<") = "<<dist_F<<" | dist_FF ("<<Ffriends[j]<<") = "<<dist_FF<<endl;
					//cout<<"triangle score = "<<pow(EULERS_NUMBER,-(dist_F + dist_FF + dist_U))<<endl;
					new_score += pow(EULERS_NUMBER,-((rpF->dist + rpFF->dist + rpU->dist)*(EARTH_CIRCUMFERENCE/360))/beta) ;

				}
			}
			
			delete Ffriends;
			FfriendsSize=0;
		}
		delete friends;
		friendsSize =0;
		my_pair user = my_pair(topkID,new_score);
		topkComplete->insert(user);
	}
	best_score = (*topkComplete->rbegin()).getScore();
	//cout <<"BEST SCORE IS NOW = "<<best_score<<endl;

	map<int, GST_User*>::iterator user_IT = GST_USER.begin();
	//cout<<"SIZE OF GST_USER = "<<GST_USER.size()<<endl;
	while(user_IT != GST_USER.end()){
		GST_User* vr = user_IT->second;	
		int vrID = user_IT->first;
		//cout<<"ID = "<<vrID<<" | score = "<<vr->score<<" | OLD COUNTER ="<<vr->counter;
		
		if(vr->score == 0){
			delete vr;
			vr = NULL;
			GST_USER.erase(user_IT++);
			//cout<<" DELETED!"<<endl;
			continue;
		}
		else{
			bool isDel=false;
			for(topkIter = topk->begin() ;topkIter!=topk->end();topkIter++){
				if(vrID==(*topkIter).getId()){
					delete vr;
					vr = NULL;
					GST_USER.erase(user_IT++);
					isDel=true;
					//cout<<" DELETED!"<<endl;
					break;
				}
			}
			if(isDel==true)
				continue;
		}
		//cout<<endl;
		
		res_point* res_vr = (points.find(vrID))->second;
		double dist_vr = res_vr->dist;	

		unsigned int possibleTriangles = (maxTriangles.find(vrID))->second;
		vr->Mvr = min(M, possibleTriangles);
		vr->Bvr = vr->score;
		
		int j = 0;
		while(vr->counter < vr->Mvr){
			vr->Bvr = vr->Bvr +  pow(EULERS_NUMBER,-((dist_vr + (MNN->at(j))->dist + DIST_LAST_USER)*(EARTH_CIRCUMFERENCE/360))/beta);
			j++;
			vr->counter =  vr->counter + 1;
		}
		//cout<<"vr->counter ("<<vrID<<") = "<<vr->counter<<" | vr->Mvr = "<<vr->Mvr<<endl; 
		
		if(vr->Bvr > best_score){
			double new_score = 0;
			spos->getFriends(vrID, friends, friendsSize);
			// user top-k - > 1 2 3 4 5 6
			
			Group* vrGP = (*groups.find(vrID)).second;
			
			for(unsigned int i = 0; i< friendsSize; i++ ){
				
				spos->getFriends(friends[i], Ffriends, FfriendsSize);
				// user 1 - > 2 4 9 10 11 12
				// user 4 - > 1 9 16 21
				for(unsigned int j = 0; j< FfriendsSize; j++ ){
					
					if(util.binarySearch(friends, i+1, friendsSize-1, Ffriends[j])){
						map<int,res_point*>::iterator iter_F = points.find(friends[i]);
						res_point* rpF; double dist_F = 0;
						res_point* rpFF; double dist_FF = 0;

						if(iter_F != points.end()){
							rpF = iter_F->second;
						}
						else{
							double userLocation[2];
							gpos->getLocation(friends[i], userLocation);
							if(userLocation[0]!=-1000){ // is not null
								dist_F =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
								rpF = util.createResultPoint(friends[i], userLocation[0], userLocation[1], dist_F);
							}
							else
								cout<<"ERROR"<<endl;
						}

						map<int,res_point*>::iterator iter_FF = points.find(Ffriends[j]);
						
						if(iter_FF != points.end()){
							rpFF = iter_FF->second;
						}
						else{
							double userLocation[2];
							gpos->getLocation(Ffriends[j], userLocation);
							if(userLocation[0]!=-1000){ // is not null
								dist_FF =  util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
								rpFF = util.createResultPoint(Ffriends[j], userLocation[0], userLocation[1], dist_FF);
								points.insert(make_pair(Ffriends[j],rpFF));
							}
							else
								cout<<"ERROR"<<endl;
						}

						vrGP->addFriend(rpF);
						vrGP->addFriend(rpFF);

						//cout<<"dist_U ("<<topkID<<") = "<<dist_U<<" | dist_F ("<<friends[i]<<") = "<<dist_F<<" | dist_FF ("<<Ffriends[j]<<") = "<<dist_FF<<endl;
						//cout<<"triangle score = "<<pow(EULERS_NUMBER,-(dist_F + dist_FF + dist_U))<<endl;
						new_score += pow(EULERS_NUMBER,-((rpF->dist + rpFF->dist + dist_vr)*(EARTH_CIRCUMFERENCE/360))/beta) ;

					}
				}
				
				delete Ffriends;
				FfriendsSize=0;
			}
			delete friends;
			friendsSize =0;
;

			if(new_score > best_score){
				my_pair user = my_pair(vrID,new_score);
				topkComplete->insert(user);
				if(topkComplete->size() > k){
					topkComplete->erase(--(topkComplete->rbegin()).base());
					best_score = (*topkComplete->rbegin()).getScore();
				}
				else if(topkComplete->size() == k){
					best_score = (*topkComplete->rbegin()).getScore();
				}
			}
		}
			
		++user_IT;	
	}
	

	// gettimeofday(&endRefinement, NULL);
	// cout<<"Total time taken for stage 2 = "<<print_time(startRefinement, endRefinement)/1000<<" ms"<<endl;
	
	//-------------------------------------------------
	//---------------end of Refinement-----------------
	//-------------------------------------------------
	

	for(topkIter = topkComplete->begin() ;topkIter!=topkComplete->end();topkIter++){
		int userID = (*topkIter).getId();
		Group* GP = (*(groups.find(userID))).second;
		GP->score = (*topkIter).getScore();
		heap->push(new Group(GP));
	}
		
	GST_USER.clear();
	gpos->clearNextNN();
	return heap;
}