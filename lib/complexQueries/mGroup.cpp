#include "../headers.h"

mGroup::mGroup(IGPOs *spatialIndex, ISPOs *socialGraph, SimpleQueries *simpleQueries){
	spos = socialGraph;
	gpos = spatialIndex;
	sq = simpleQueries;

	usersChecked = 0;
	current_dist = 0.0;

	computeLBTime = 0.0;

}


mGroup::~mGroup(){

}


int mGroup::getNumOfUsersChecked(){

	return usersChecked;

}


double mGroup::computeGroupLBNew(vector<Group*>* seen_groups, int m){

        double lb = current_dist;
        int x = 1;
        int checked = 0;
        int d = current_dist*100000000;

        for(int i = 0; i < (int)seen_groups->size() && x <= m-1; i++){
                if((*seen_groups)[i]->longest_dist == current_dist){ // otherwise they cannot be friends, see the lemma
			lb+=(*seen_groups)[i]->user_dist;
                        x++;
                        (*seen_groups)[i]->updateLongestDist(current_dist);
                }

                checked++;
        }

        if(x-1 < m)
                lb+=((m-x)*current_dist);

        return lb;
}



// Computes the LB for an unseen user. In the best case, his group contains the m-1 closest users to q.
double mGroup::computeGroupLB(res_point* user, vector<res_point*>* seen_users, int m, char* f){

	if(strcmp(f, "max") == 0)
		return user->dist;
	else{ // sum

		double lb = user->dist;
		vector<res_point*>::iterator i;

		int x = 1;
		res_point* tmp = NULL;

		for(i=seen_users->begin(); i != seen_users->end() && x <= m-1; ++i, ++x){
			tmp = *i;
			lb += tmp->dist;
		}

		//if we have not found the m-1 closest users to q yet
		if(x-1 < m)
			lb+= (m-x)*tmp->dist;  

		return lb;
	}
}



// Simple query spatial expansion approach. It checks all users we have seen yet.
priority_queue <Group*, vector<Group*>, Group::descending >* mGroup::LAZY(double x, double y, int m, int k, int incrStep, char* f){
     
     	double best_dist = 999999;
     	map<int, Group*>* seen_groups = new map<int, Group*>();
	vector<res_point*>* seen_users = new vector<res_point*>();
	priority_queue <Group*, vector<Group*>, Group::descending >* heap = new priority_queue <Group*, vector<Group*>, Group::descending >();
//	priority_queue<Group*>* heap;// = new priority_queue <Group*, vector<Group*>, Group::descending >();
//	if(strcmp(f, "sum") == 0)
//		heap = new priority_queue <Group*, vector<Group*>, Group::descending >();
//	else
//		heap = new priority_queue <Group*, vector<Group*>, Group::descending >();

     	res_point* tmp;   
	int heap_size = 0;     
     	res_point* newP;
	Group* newG;
	int askNext = 0;
	int computedNN = 0;
	int j = 0;
	int* friends;
	unsigned int friendsSize;
	double group_best_dist = 9999999;
	double heap_best_dist = 9999999;
	
	//cout << "Lazy start in" << endl;

	newP = gpos->getNextNN(x, y, incrStep);
	seen_users->push_back(newP);
	//cout << "Lazy: get Next finished" << endl;

	newG = new Group(newP, m, f);   
	newG->lb = computeGroupLB(newP, seen_users, m, f);
	seen_groups->insert(pair<int, Group*>(newP->id, newG));
	int q = 1;

     	while((newG->lb < best_dist) && newP != NULL  && heap->size() <= k){
	//	cout << q << ") user = " << newP->id << "\t dist = " << newP->dist << "\t best_dist = " << best_dist << "lb = " << newG->lb << endl;
		spos->getFriends(newP->id, friends, friendsSize);
          	for(int i = 0; i < (int)seen_users->size(); ++i){
              		tmp = (*seen_users)[i];
			if(util.binarySearch(friends, 0, friendsSize, tmp->id)){
				if(newG->size() < m){
                        		newG->addFriend(tmp);

					if(strcmp(f, "max") == 0){
						group_best_dist = newG->max_dist;
					}
					else{ // sum
						group_best_dist = newG->adist;
					}

                        		if(newG->size() == m && group_best_dist < best_dist){
						heap->push(new Group(newG));
						heap_size++;

						if(heap_size == k){
							if(strcmp(f, "max") == 0){
								heap_best_dist = heap->top()->max_dist;;
							}
							else{ // sum
								heap_best_dist = heap->top()->adist;;
							}

							best_dist = heap_best_dist;
						}
						else if(heap_size > k){
						//	delete heap->top(); ---------------------------------------------------------------
							heap->pop();
							heap_size--;
							if(strcmp(f, "max") == 0){
								heap_best_dist = heap->top()->max_dist;;
							}
							else{ // sum
								heap_best_dist = heap->top()->adist;;
							}

							best_dist = heap_best_dist;
						}
       						//best_dist = heap->top()->adist;
                        		} 
                  		}

                  		map<int,Group*>::iterator it;
                  		it = seen_groups->find(tmp->id);
                  		if(it != seen_groups->end()){
                        		Group* tmpG = (*it).second;
                        		if(tmpG->size() < m){
                                		tmpG->addFriend(newP);

						if(strcmp(f, "max") == 0){
							group_best_dist = tmpG->max_dist;
						}
						else{ // sum
							group_best_dist = tmpG->adist;
						}


                                		if(tmpG->size() == m && group_best_dist < best_dist){

							heap->push(new Group(tmpG));
							heap_size++;

							if(heap_size == k){
								if(strcmp(f, "max") == 0){
									heap_best_dist = heap->top()->max_dist;;
								}
								else{ // sum
									heap_best_dist = heap->top()->adist;;
								}

								best_dist = heap_best_dist;
							}
							else if(heap_size > k){
							//	delete heap->top();   ----------------------------------------------------------
								heap->pop();
								heap_size--;
								if(strcmp(f, "max") == 0){
									heap_best_dist = heap->top()->max_dist;;
								}
								else{ // sum
									heap_best_dist = heap->top()->adist;;
								}

			              				best_dist = heap_best_dist; 
							}
       							//best_dist = heap->top()->adist;
                                		}
                        		}
                  		}
              		}
          	}

		newP = gpos->getNextNN(x, y, incrStep);
		usersChecked++;
		if(newP != NULL){
			seen_users->push_back(newP);
		  	newG = new Group(newP, m, f);
			newG->lb = computeGroupLB(newP, seen_users, m, f);
	     		seen_groups->insert(pair<int, Group*>(newP->id, newG));
		}
		q++;
		group_best_dist = heap_best_dist = 9999999;
     	}

//	if(newP == NULL)
//		cout << "newP is NULL !!!!!!!!!!!!" << endl;

/*	for(std::map<int, Group*>::iterator MapItor = seen_groups->begin(); MapItor != seen_groups->end(); ++MapItor)
	{
		Group* Value = (*MapItor).second;
		delete Value;
	}
	delete seen_groups;
*/
	gpos->clearNextNN();

//	cout << "userschecked = " << usersChecked << endl;

	return heap;
}






priority_queue <Group*, vector<Group*>, Group::descending >* mGroup::EAGER(double x, double y, int m, int k, int incrStep, char* f){

	double best_dist = 999999;
     	double lb = 0;
     	res_point* newP;
	Group* tmpG;
	priority_queue <Group*, vector<Group*>, Group::descending >* heap = new priority_queue <Group*, vector<Group*>, Group::descending >();
	int heap_size = 0;
	vector<res_point*>* seen_users = new vector<res_point*>();

	double group_best_dist = 9999999;
        double heap_best_dist = 9999999;

	newP = gpos->getNextNN(x, y, incrStep);
	seen_users->push_back(newP);
     	lb = computeGroupLB(newP, seen_users, m, f);

//	cout << "test1" << endl;

     	while(lb < best_dist && newP != NULL && heap->size() <= k){
          	tmpG = sq->getClosestFriends3(x, y, newP, m, f);
//		cout << "user = " << newP->id << "\t dist = " << newP->dist << "\t best_dist = " << best_dist << endl;

		if(tmpG!=NULL){

			if(strcmp(f, "max") == 0){
                        	group_best_dist = tmpG->max_dist;
                        }
                        else{ // sum
                                group_best_dist = tmpG->adist;
                        }

//			tmpG->print();
//			cout << "\t group_dist = " << group_best_dist << endl;

		  	if (best_dist > group_best_dist){

				heap->push(tmpG);
				heap_size++;
				if(strcmp(f, "max") == 0){
                                	heap_best_dist = heap->top()->max_dist;;
                                }
                                else{ // sum
                                        heap_best_dist = heap->top()->adist;;
                                }


				if(heap_size == k){
					best_dist = heap_best_dist;
//					cout << "change best_dist = " << best_dist << endl;
				}
				else if(heap_size > k){
					delete heap->top();

					heap->pop();
					heap_size--;

					if(strcmp(f, "max") == 0){
                               	 		heap_best_dist = heap->top()->max_dist;;
                                	}
                               		else{ // sum
                                        	heap_best_dist = heap->top()->adist;;
                                	}



		      			best_dist = heap_best_dist; 
//					cout << "change best_dist = " << best_dist << endl;
				}
          		}
			else
				delete tmpG;
		}

		newP = gpos->getNextNN(x, y, incrStep);
		if(newP != NULL){
			seen_users->push_back(newP);
          		lb = computeGroupLB(newP, seen_users, m, f);   
		}
		usersChecked++;
		group_best_dist = heap_best_dist = 9999999;
	//	cout << endl;
     	}


//	cout << "userschecked = " << usersChecked << endl;

//	cout << "Test end" << endl;
	gpos->clearNextNN();
//	cout << "Test end" << endl;

	return heap;
}



priority_queue <Group*, vector<Group*>, Group::descending >* mGroup::EAGER_OPT(double x, double y, int m, int k, int incrStep){
	clock_t startC, endC;
        double best_dist = 999999;
        double lb = 0;
        res_point* newP;
        Group* tmpG;
        priority_queue <Group*, vector<Group*>, Group::descending >* heap = new priority_queue <Group*, vector<Group*>, Group::descending >();
        vector<Group*>* seen_groups = new vector<Group*>();
        map<int, Group*>* seen_friends = new map<int, Group*>();
        int heap_size = 0;
        vector<res_point*> tmp;
        map<int, Group*>::iterator it;
        int i = 0;
        res_point* c;
        Group* t;
	char f[] = "sum";
	vector<int>* gids = new vector<int>();
	set<int>::iterator itset; 
	int limit = 0;
	set<int> heapIds;

	startC = clock();
//	tmpG = sq->getFirstmGroup(x, y, m, k);
	double startDist = sq->getFirstmGroup(x, y, m, k);

        newP = gpos->getNextNN(x, y, incrStep);
        current_dist = newP->dist;


	// 13/4/2013        lb = computeGroupLBNew(seen_groups, m);   
        lb = m*current_dist;

	//tmpG = sq->getClosestFriends3(x, y, newP, m); 
        tmpG = sq->getMyFriendsInRange3(x, y, startDist, newP, m);
	while(lb < best_dist && newP!=NULL  && heap->size() <= k){ 
	//	cout << "user = " << newP->id << "\t dist = " << newP->dist << "\t best_dist = " << best_dist << endl;

                if(tmpG != NULL){

                        seen_groups->push_back(tmpG);

                        while(!tmpG->friends->empty()) {
                                c = tmpG->friends->top();
                                tmp.push_back(c);
				itset = heapIds.find(c->id);
				if(itset == heapIds.end()){
                                	it = seen_friends->find(c->id);
	                                if(it!=seen_friends->end()){ // we have seen him already
        	                                t = it->second;
                	                        if(t->size() < m && t->id != tmpG->id){
                        	                        res_point* rp = util.createResultPoint(tmpG->id, 0, 0, tmpG->user_dist);
                                	                t->addFriend(rp);

                                        	        if(t->size() == m && t->adist < best_dist){           
                                                	        heap->push(new Group(t));
                                                       		seen_friends->erase(c->id);                   // we should not but let's see
	                                                        heap_size++; 
							//	gids->insert(gids->begin(), t->id);
								heapIds.insert(t->id);
							//	t->print();
							//	cout << "- ins (f) " << t->id << "\t" << t->adist << endl;
                                	                        if(heap_size == k){
                                        	                        best_dist = heap->top()->adist;
                                                	        }
                                                        	else if(heap_size > k){
                                                                	//delete heap->top();
	                                                                heap->pop();
        	                                                        heap_size--;
                	                                                best_dist = heap->top()->adist; 
                        	                                }
	                                                }
        	                                }

                			}
                                	else{ // first time we meet him
                                        	Group* newG = new Group(c, m, f);
                                        	newG->addFriend(util.createResultPoint(tmpG->id, 0, 0, tmpG->user_dist));       
                                        	seen_friends->insert(pair<int, Group*>(c->id, newG));
                                	}
				}
                                tmpG->friends->pop();

                        }

                        for(int i = 0; i < tmp.size(); i++){
                                tmpG->friends->push(tmp[i]);
                        }

                        tmp.clear();

                        if (tmpG->size() >= m && best_dist > tmpG->adist){

                                heap->push(tmpG);
                                heap_size++;
				seen_friends->erase(tmpG->id); 		// 7/11/2012
			//	cout << "- ins (g) " << tmpG->id << "\t" << tmpG->adist << endl;
				heapIds.insert(tmpG->id);
			//	tmpG->print();
			//	gids->insert(gids->begin(), tmpG->id);

                                if(heap_size == k){
                                        best_dist = heap->top()->adist;
                                }
                                else if(heap_size > k){
                                //      delete heap->top();

                                        heap->pop();
                                        heap_size--;
                                        best_dist = heap->top()->adist; 
                                }       

                        }
                }

                newP = gpos->getNextNN(x, y, incrStep);

                if(newP!=NULL){

                        current_dist = newP->dist;
                        // 13/4/2013 lb = computeGroupLBNew(seen_groups, m); 
			lb = m*current_dist;  
                 	itset = heapIds.find(newP->id);
			if(itset == heapIds.end())
				tmpG = sq->getMyFriendsInRange3(x, y, best_dist, newP, m);  // ---------------------------- here we need to give m for the adist!
			else
				tmpG = NULL;
                }
                usersChecked++;
                i++;
		//cout << "i = " << i << endl;
        }

	endC = clock();
        computeLBTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));


//	for(int i=0; i < gids->size(); i++)
//		cout << (*gids)[i] << " ";
//	cout << endl;

//	cout << "----" ;

//	limit = gids->size();



//	cout << "heap_size = " << heap_size << endl;


        for(it = seen_friends->begin(); it != seen_friends->end(); it++){
                t = it->second;
		itset = heapIds.find(t->id);
                if((t->adist+current_dist*(m-t->size())) <= best_dist && t->size() < m && itset == heapIds.end()){                // otherwise you do not need to check it out!
                        struct res_point* n = (struct res_point*) malloc(sizeof(struct res_point));
                        n->id = t->id;
                        n->dist = t->user_dist;
                        tmpG = sq->getClosestFriends3(x, y, n, m);

                        if(tmpG != NULL){
                                if (best_dist > tmpG->adist){

                                        heap->push(tmpG);
                                        heap_size++;
					gids->insert(gids->begin(), tmpG->id);
			//		cout << "- ins (2r) " << tmpG->id << "\t" << tmpG->adist << endl;
					heapIds.insert(tmpG->id);
					//tmpG->print();

                                        if(heap_size == k){
                                                best_dist = heap->top()->adist;
                                        }
                                        else if(heap_size > k){
                                        //        delete heap->top();
                                                heap->pop();
                                                heap_size--;
                                                best_dist = heap->top()->adist; 
                                        }

                                }
                                else
                                        delete tmpG;
                        }
                        usersChecked++;
                        free(n);                        //------------------------------------------------------ we should do it

                }

        }



        gpos->clearNextNN();
        vector<Group*> empty;
        seen_groups->swap(empty);


        for(std::map<int, Group*>::iterator MapItor = seen_friends->begin(); MapItor != seen_friends->end(); ++MapItor)
        {
                Group* Value = (*MapItor).second;
                delete Value;
        }
        delete seen_friends;

/*	for(int i=limit; i < gids->size(); i++)
		cout << (*gids)[i] << " ";
	cout << endl;
*/

        return heap;
}





priority_queue <Group*, vector<Group*>, Group::descending >* mGroup::EAGER_OPT_RANGE(double x, double y, int m, int k, int incrStep){
	clock_t startC, endC;
        double best_dist = 999999;
        double lb = 0;
        res_point* newP;
        Group* tmpG = NULL;
        priority_queue <Group*, vector<Group*>, Group::descending >* heap = new priority_queue <Group*, vector<Group*>, Group::descending >();
        vector<Group*>* seen_groups = new vector<Group*>();
        map<int, Group*>* seen_friends = new map<int, Group*>();
        int heap_size = 0;
        int* friends;
        unsigned int friendsSize;
        vector<res_point*> tmp;
        map<int, Group*>::iterator it;
        int i = 0;
        res_point* c;
        Group* t;
	vector<res_point*>* usersInValidRange;// = gpos->getRangeSortedId(x, y, lb);
	char f[] = "sum";
	startC = clock();
	set<int>::iterator itset; 
	int limit = 0;
	set<int> heapIds;


	//returns the mGroup of the closest user to q who has an mGroup
	double startDist = sq->getFirstmGroup(x, y, m, k);
//	cout << "First mGroup = " << tmpG->id << "\t adist = " << tmpG->adist << "\t user dist = " << tmpG->user_dist << endl;

//	cout << "best distance = " << best_dist << endl;

        newP = gpos->getNextNN(x, y, incrStep);

//        current_dist = newP->dist;
//        cout << "Distance of the first = " << newP->dist << endl;

//        lb = computeGroupLBNew(seen_groups, m);   


	usersInValidRange = gpos->getRange(x, y, startDist);

//	cout << "startDist = " << startDist << endl;
//	cout << "usersInValidRange = " << usersInValidRange->size() << endl;

        while(lb <= best_dist && newP!=NULL  && heap->size() <= k){ 
//		cout << "user = " << newP->id << "\t dist = " << newP->dist << "\t best_dist = " << best_dist << endl;

                        current_dist = newP->dist;
                        lb = computeGroupLBNew(seen_groups, m);
		

			itset = heapIds.find(newP->id);
                        if(itset == heapIds.end()){
				spos->getFriends(newP->id, friends, friendsSize);
//				if(newP->id == 37679255)
//					cout << "------------------------------- 37679255 friends# = " << friendsSize;
//				cout << "computeMyGroup" << endl;
				tmpG = sq->computeMyGroup(usersInValidRange, friends, friendsSize, newP, m);
//				cout << "computeMyGroup" << endl;
				friendsSize = 0;
			}
			else
				tmpG = NULL;

//		tmpG = sq->computeMyGroup(usersInValidRange, friends, friendsSize, newP, m);

                if(tmpG != NULL){ 
//			tmpG->print();
//			cout << "test0.1" << endl;
                       seen_groups->push_back(tmpG);
//			cout << "test0.2" << endl;

			if(tmpG->friends->size()>0){
//				cout << "test0.3" << endl;

//				cout << "# friends = " << tmpG->friends->size();
        	                while(!tmpG->friends->empty()) {
//					cout << "test1" << endl;
                        	        c = tmpG->friends->top();
                                	tmp.push_back(c);
  					itset = heapIds.find(c->id);
					if(itset == heapIds.end()){
		                                it = seen_friends->find(c->id);
        		                        if(it!=seen_friends->end()){ // we have seen him already
                		                        t = it->second;
                        		                if(t->size() < m && t->id != tmpG->id){
                                		                res_point* rp = util.createResultPoint(tmpG->id, 0, 0, tmpG->user_dist);
                                        		        t->addFriend(rp);

                                                		if(t->size() == m && t->adist <= best_dist){           
                                                        		heap->push(new Group(t));
	                                                        	seen_friends->erase(c->id);                   // we should not but let's see
	        	                                                heap_size++; 
									heapIds.insert(t->id);


                		                                        if(heap_size == k){
                        		                                      //  if(best_dist == 999999){
										//	best_dist = heap->top()->adist;
										//	usersInValidRange = gpos->getRange(x, y, best_dist);
										//	cout << "best_dist = " << best_dist << "\t # = " << usersInValidRange->size() << endl;
									//	}
									//	else{
										best_dist = heap->top()->adist;
										util.updateUsersInValidRange(usersInValidRange, best_dist);
									//	}
                                	                        	}
	                                        	                else if(heap_size > k){
        	                                        	                //delete heap->top();
                	                                        	        heap->pop();
                        	                                        	heap_size--;
	                        	                                        best_dist = heap->top()->adist;
										util.updateUsersInValidRange(usersInValidRange, best_dist);
                	                        	                }
	
        	                                        	}
                	                        	}
	
        	                        	}
	                	                else{ // first time we meet him
//							cout << "test2" << endl;
	        	                	        Group* newG = new Group(c, m, f);
                        	                	newG->addFriend(util.createResultPoint(tmpG->id, 0, 0, tmpG->user_dist));       
	                        	                seen_friends->insert(pair<int, Group*>(c->id, newG));
//							cout << "test2 end" << endl;

                	                	}
//						cout << "Friend -> " << tmpG->friends->top()->id << endl;
                                	}
					tmpG->friends->pop();

                        	}

	                        for(int i = 0; i < tmp.size(); i++){
        	                        tmpG->friends->push(tmp[i]);
                	        }

                        	tmp.clear();
			} // if he does not have friends

//			cout << "test3" << endl;

                        if (tmpG->size() >= m && best_dist >= tmpG->adist){

                                heap->push(tmpG);
                                heap_size++;
				heapIds.insert(tmpG->id);
				seen_friends->erase(tmpG->id); 		// 7/11/2012


                                if(heap_size == k){
				//	if(best_dist == 999999){
                                //        	best_dist = heap->top()->adist;
                                //                usersInValidRange = gpos->getRange(x, y, best_dist);
				//		cout << "best_dist = " << best_dist << "\t # = " << usersInValidRange->size() << endl;
                                //        }
                                //        else{
                                                best_dist = heap->top()->adist;
                                                util.updateUsersInValidRange(usersInValidRange, best_dist);
                                //        }
                                }
                                else if(heap_size > k){
                                //      delete heap->top();
                                        heap->pop();
                                        heap_size--;
                                        best_dist = heap->top()->adist; 
					util.updateUsersInValidRange(usersInValidRange, best_dist);
                                }       

                        }
                }

                newP = gpos->getNextNN(x, y, incrStep);


/*                if(newP!=NULL){

                        current_dist = newP->dist;
                        lb = computeGroupLBNew(seen_groups, m);
			spos->getFriends(newP->id, friends, friendsSize);
			cout << "computeMyGroup" << endl;
			tmpG = sq->computeMyGroup(usersInValidRange, friends, friendsSize, newP, m);
			cout << "computeMyGroup" << endl;
			friendsSize = 0;

                }
*/  
//  	      friends = NULL;

              usersChecked++;
                i++;         
        }     

	endC = clock();
        computeLBTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));


//	cout << "------------------------- Second phase -------------------------------" << endl;


	if(seen_friends != NULL && seen_friends->size()>0){

      		for(it = seen_friends->begin(); it != seen_friends->end(); it++){
                	t = it->second;
			itset = heapIds.find(t->id);
			if(t!=NULL && itset == heapIds.end()){
             	   		if((t->adist+current_dist*(m-t->size())) <= best_dist && t->size() < m){                // otherwise you do not need to check it out!
        	                	struct res_point* n = (struct res_point*) malloc(sizeof(struct res_point));
	                	        n->id = t->id;
		                        n->dist = t->user_dist;
					spos->getFriends(n->id, friends, friendsSize);
					tmpG = sq->computeMyGroup(usersInValidRange, friends, friendsSize, n, m);

                		        if(tmpG != NULL){
                        		        if (best_dist >= tmpG->adist){
	
		                                        heap->push(tmpG);
        		                                heap_size++;
							heapIds.insert(tmpG->id);

        	        	                        if(heap_size == k){
                	        	                        best_dist = heap->top()->adist;
                       		        			util.updateUsersInValidRange(usersInValidRange, best_dist);
						        }
                                	        	else if(heap_size > k){
                                        	        	delete heap->top();
	                                        	        heap->pop();
        	                                        	heap_size--;
	                	                                best_dist = heap->top()->adist; 
								util.updateUsersInValidRange(usersInValidRange, best_dist);
        	                	                }
                	                	}
	                	                else
        	                	                delete tmpG;
	                	        }
        	                	usersChecked++;
				}
                	}
		}
        }

//	cout << "End of Second Phase 0" << endl;


        gpos->clearNextNN();

//	cout << "End of Second Phase 1" << endl;


	if(seen_groups!=NULL){
		vector<Group*> empty;
        	seen_groups->swap(empty);
	}

//	cout << "End of Second Phase 3" << endl;


	if(seen_friends !=NULL){

        	for(std::map<int, Group*>::iterator MapItor = seen_friends->begin(); MapItor != seen_friends->end(); ++MapItor)
        	{
                	Group* Value = (*MapItor).second;
                	delete Value;
        	}       
        	delete seen_friends;
	}

//	cout << "End of Second Phase 4" << endl;


        return heap;
}




priority_queue <Group*, vector<Group*>, Group::descending >* mGroup::EAGER_OPT_FINAL(double x, double y, int m, int k, int incrStep){
	clock_t startC, endC;
        double best_dist = 999999;
        double lb = 999999;
        res_point* newP;
        Group* tmpG;
        priority_queue <Group*, vector<Group*>, Group::descending >* heap = new priority_queue <Group*, vector<Group*>, Group::descending >();
        vector<Group*>* seen_groups = new vector<Group*>();
        map<int, Group*>* seen_friends = new map<int, Group*>();
        int heap_size = 0;
        vector<res_point*> tmp;
        map<int, Group*>::iterator it;
        int i = 0;
        res_point* c;
        Group* t;
	char f[] = "sum";
	vector<int>* gids = new vector<int>();
	set<int>::iterator itset; 
	int limit = 0;
	set<int> heapIds;

	startC = clock();
//	tmpG = sq->getFirstmGroup(x, y, m, k);
	double startDist = sq->getFirstmGroup(x, y, m, k);

	best_dist = startDist;

        newP = gpos->getNextNN(x, y, incrStep);
        current_dist = newP->dist;


        //lb = 999999//computeGroupLBNew(seen_groups, m);   
        //tmpG = sq->getClosestFriends3(x, y, newP, m); 
//        tmpG = sq->getMyFriendsInRange3(x, y, startDist, newP, m);
	
//	best_dist = tmpG->adist;
//	heap->push(new Group(tmpG));
//	heap_size++;

	while(lb < best_dist && newP!=NULL  && heap->size() <= k){ 
	//	cout << "user = " << newP->id << "\t dist = " << newP->dist << "\t best_dist = " << best_dist << endl;
		
	    //    tmpG = sq->getMyFriendsInRange3(x, y, best_dist, newP, m);


                if(tmpG != NULL){

			if (tmpG->size() >= m && best_dist > tmpG->adist){

                                heap->push(tmpG);
                                heap_size++;
				seen_friends->erase(tmpG->id); 		// 7/11/2012
			//	cout << "- ins (g) " << tmpG->id << "\t" << tmpG->adist << endl;
				heapIds.insert(tmpG->id);
			//	tmpG->print();
			//	gids->insert(gids->begin(), tmpG->id);

                                if(heap_size == k){
                                        best_dist = heap->top()->adist;
                                }
                                else if(heap_size > k){
                                //      delete heap->top();

                                        heap->pop();
                                        heap_size--;
                                        best_dist = heap->top()->adist; 
                                }       

                        }                    


                        while(!tmpG->friends->empty()) {
                                c = tmpG->friends->top();
                                tmp.push_back(c);
				itset = heapIds.find(c->id);
				if(itset == heapIds.end()){
                                	it = seen_friends->find(c->id);
	                                if(it!=seen_friends->end()){ // we have seen him already
        	                                t = it->second;
                	                        if(t->size() < m && t->id != tmpG->id){
                        	                        res_point* rp = util.createResultPoint(tmpG->id, 0, 0, tmpG->user_dist);
                                	                t->addFriend(rp);

                                        	        if(t->size() == m && t->adist < best_dist){           
                                                	        heap->push(new Group(t));
                                                       		seen_friends->erase(c->id);                   // we should not but let's see
	                                                        heap_size++; 
							//	gids->insert(gids->begin(), t->id);
								heapIds.insert(t->id);
							//	t->print();
							//	cout << "- ins (f) " << t->id << "\t" << t->adist << endl;
                                	                        if(heap_size == k){
                                        	                        best_dist = heap->top()->adist;
                                                	        }
                                                        	else if(heap_size > k){
                                                                	//delete heap->top();
	                                                                heap->pop();
        	                                                        heap_size--;
                	                                                best_dist = heap->top()->adist; 
                        	                                }
	                                                }
        	                                }

                			}
                                	else{ // first time we meet him
                                        	Group* newG = new Group(c, m, f);
                                        	newG->addFriend(util.createResultPoint(tmpG->id, 0, 0, tmpG->user_dist));       
                                        	seen_friends->insert(pair<int, Group*>(c->id, newG));
                                	}
				}
                                tmpG->friends->pop();

                        }

                        for(int i = 0; i < tmp.size(); i++){
                                tmpG->friends->push(tmp[i]);
                        }

                        tmp.clear();

                        
                }

                newP = gpos->getNextNN(x, y, incrStep);

                if(newP!=NULL){

                        current_dist = newP->dist;
			lb = m*current_dist;
                 //       lb = computeGroupLBNew(seen_groups, m);   
                 	itset = heapIds.find(newP->id);
			if(itset == heapIds.end())
				tmpG = sq->getMyFriendsInRange3(x, y, best_dist, newP, m);  // ---------------------------- here we need to give m for the adist!
			else
				tmpG = NULL;
                }
                usersChecked++;
                i++;
		//cout << "i = " << i << endl;
        }

	endC = clock();
        computeLBTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));


//	for(int i=0; i < gids->size(); i++)
//		cout << (*gids)[i] << " ";
//	cout << endl;

//	cout << "----" ;

//	limit = gids->size();



//	cout << "heap_size = " << heap_size << endl;


        for(it = seen_friends->begin(); it != seen_friends->end(); it++){
                t = it->second;
		itset = heapIds.find(t->id);
                if((t->adist+current_dist*(m-t->size())) <= best_dist && t->size() < m && itset == heapIds.end()){                // otherwise you do not need to check it out!
                        struct res_point* n = (struct res_point*) malloc(sizeof(struct res_point));
                        n->id = t->id;
                        n->dist = t->user_dist;
                        tmpG = sq->getClosestFriends3(x, y, n, m);

                        if(tmpG != NULL){
                                if (best_dist > tmpG->adist){

                                        heap->push(tmpG);
                                        heap_size++;
					gids->insert(gids->begin(), tmpG->id);
			//		cout << "- ins (2r) " << tmpG->id << "\t" << tmpG->adist << endl;
					heapIds.insert(tmpG->id);
					//tmpG->print();

                                        if(heap_size == k){
                                                best_dist = heap->top()->adist;
                                        }
                                        else if(heap_size > k){
                                        //        delete heap->top();
                                                heap->pop();
                                                heap_size--;
                                                best_dist = heap->top()->adist; 
                                        }

                                }
                                else
                                        delete tmpG;
                        }
                        usersChecked++;
                        free(n);                        //------------------------------------------------------ we should do it

                }

        }



        gpos->clearNextNN();
        //vector<Group*> empty;
       // seen_groups->swap(empty);


        for(std::map<int, Group*>::iterator MapItor = seen_friends->begin(); MapItor != seen_friends->end(); ++MapItor)
        {
                Group* Value = (*MapItor).second;
                delete Value;
        }
        delete seen_friends;

/*	for(int i=limit; i < gids->size(); i++)
		cout << (*gids)[i] << " ";
	cout << endl;
*/

        return heap;
}




