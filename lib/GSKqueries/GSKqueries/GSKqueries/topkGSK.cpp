#include "../headersMemory.h"
#include <unordered_map>
topkGSK::topkGSK(IGPOs *spatialIndex, ISPOs *socialGraph, InvertedIndexGen *ivgenerator , SimpleQueries *simpleQueries){
	spos = socialGraph;
	gpos = spatialIndex;
	sq = simpleQueries;
	ivgen = ivgenerator;

	usersChecked = 0;
	threshold = 0.0;
}

topkGSK::~topkGSK(){}

double topkGSK::print_time(struct timeval &start, struct timeval &end){
    double usec;
    usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    return usec;
}

/*
Problem: Given a spatial location q, a set of keywords K, and a positive integer k, get the top-k users based on their textual similarity to K, their degree and their distance to q.

Naive Solution: for all users a) compute their distance to q, b) compute their textual similarity to K, c) compute their degree (precomputed), d) combine their sub-scores, rank them, and return the top-k.

Better Solution: Threshold algorithm: 
•List 1 keeps the users in sorted distance from the query location (next nearest user)
•List 2 keeps the users sorted in descending order of their degree (precomputed)
•List 3: For each keyword in K, get the sorted list with the ids of the users whose description contains the corresponding keyword. Then follow the method that I mentioned in order to return the user ids based on their appearance in the lists.

Better solution 2: Use a hybrid index that combines users' degree along with spatial information. 
*/


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::naiveAlgorithm(double x, double y, unsigned int k, double w, vector<string> *terms){
	
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
		
	double best_score = -1;
	unsigned int heap_size = 0;
	
	//get all users sorted by distance in vector
	vector<res_point*>* allUsers =  gpos->getkNN(x, y, DATASET_SIZE);
	vector <res_point*>::iterator itUsers;
	// cout <<"ALL USERS SIZE = "<<allUsers->size();
	
	unordered_map<int,double> keywordMap;
	
	for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
		double tfidf = 0;
		vector<tf_pair>* posting_list = ivgen->getUsersWithTFfromVEC(*it);		
		for(std::vector<tf_pair>::iterator iter = posting_list->begin(); iter != posting_list->end(); ++iter) {
			
			tf_pair tf_user = *iter;
			int user_id = tf_user.getId();
			int term_frequency = tf_user.getTF();
			double idf = log10(DATASET_SIZE/(1 + posting_list->size()));
			double local_tfidf = term_frequency * idf;
			
			// cout<<" user ID = " << user_id <<" local tfidf = "<<local_tfidf <<" term_freq = "<<term_frequency<<" idf = "<< idf <<endl;
			
			auto it_id = keywordMap.find(user_id);
			
			if(it_id!= keywordMap.end()){ 
				tfidf = local_tfidf + it_id->second;
				keywordMap[user_id]=tfidf;
			}
			else{
				keywordMap[user_id]=local_tfidf;
			}
		}
	}
	
	
	for(itUsers = allUsers->begin(); itUsers != allUsers->end(); ++itUsers){
		res_point* newP = *itUsers;
		//social score
		int user_degree = spos->getUserDegree(newP->id);
		double social_score = (double)user_degree/(double)MAXSC;
		
		//textual score
		// double tfidf = 0;
		// for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
			// tfidf += ivgen->calculateTFIDFfromSET(*it, newP->id);
		// }
		
		double tfidf = 0;
		unordered_map<int,double>::iterator it = keywordMap.find(newP->id);
		if(it != keywordMap.end()){
			tfidf = (*it).second;
		}
		
		//geographical score
		double geographical_score = 1 - (newP->dist / (double)MAXDIST);
		
		Group* newG = new Group(newP);
		newG->score = social_score + tfidf + geographical_score;
		
		// cout << "user = " << newP->id << "\t dist = " << newP->dist <<" | "<<(EARTH_CIRCUMFERENCE/360) * newP->dist<<"km | Score = "<< newG->score <<endl;

		//keep the top k in the priority queue
		if(newG->score >= best_score){
			heap->push(newG);
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
	
	return heap;
}



priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::SimpleTA(double x, double y, unsigned int k, int incrStep , double w, vector<string> *terms){
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	
	unsigned int heap_size = 0;
	double threshold_value = -1;
	double last_soc_score  = 0;
	double last_geo_score  = 0;
	double last_key_score  = 0;
	
	multiset<my_pair,pair_comparator_descending> topk ;
	
	// vector<res_point*>* usersGEO =  gpos->getNextNN(x, y, incrStep);
	// vector <res_point*>::iterator itGEO = usersGEO->begin();
	// cout<<"usersGEO size = " <<usersGEO->size() << endl;
	res_point* newP;
	
	
	multiset<my_pair,pair_comparator_descending>* degreeSet = spos->getDegreeSet();
	multiset<my_pair,pair_comparator_descending>::iterator itSOC = degreeSet->begin();
	// cout<<"degreeSet size = " <<degreeSet->size() << endl;
	
	multiset<my_pair,pair_comparator_descending>* keywordSet = new multiset<my_pair,pair_comparator_descending>();
	unordered_map<int,double> keywordMap;
	unordered_map<int, multiset< my_pair,pair_comparator_descending >::iterator> quick_search_map_key;

	double tfidf = 0;
	for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {

		vector<tf_pair>* posting_list = ivgen->getUsersWithTFfromVEC(*it);		
		for(std::vector<tf_pair>::iterator iter = posting_list->begin(); iter != posting_list->end(); ++iter) {
			
			tf_pair tf_user = *iter;
			int user_id = tf_user.getId();
			int term_frequency = tf_user.getTF();
			double idf = log10(DATASET_SIZE/(1 + posting_list->size()));
			double local_tfidf = term_frequency * idf;
			
			// cout<<" user ID = " << user_id <<" local tfidf = "<<local_tfidf <<" term_freq = "<<term_frequency<<" idf = "<< idf <<endl;
			
			auto it_id = quick_search_map_key.find(user_id);
				
			if(it_id!= quick_search_map_key.end()){ 
				multiset<my_pair,pair_comparator_descending>::iterator l_it = it_id->second;
				tfidf = local_tfidf + (*l_it).getScore();
				keywordMap[user_id]=tfidf;
				keywordSet->erase(l_it);
				keywordSet->insert(my_pair(user_id,tfidf));
				
			}
			else{
				keywordMap[user_id]=local_tfidf;
				quick_search_map_key.insert(make_pair(user_id,keywordSet->insert(my_pair(user_id,local_tfidf))));
			}
		}
	}
	
	
	// cout<<"keywordSet size = " <<keywordSet->size() << endl;
	// multiset<my_pair,pair_comparator_descending>::iterator asd;
	// for(asd = keywordSet->begin(); asd != keywordSet->end(); asd++){
		// my_pair user = *asd;
		// cout<<" user ID = " << user.getId() <<" local tfidf = "<<user.getScore()<<endl;
	// }
	
	
	multiset<my_pair,pair_comparator_descending>::iterator itKEY = keywordSet->begin();
	
	//social: use degreeSet for sequential access and spos->getuserDegree(x) for random access
	//geographical: use getUserLocation and calculate dist for random access. getkNN for sequential access
	
	set<int> seen_users;
	set<int>::iterator ITseen;
	newP = gpos->getNextNN(x, y, incrStep);
	int round_robin = 0;
	
	while(heap_size != k && newP != NULL && itSOC!=degreeSet->end()){
		bool flag = false;
		double local_geo_score = 0, local_soc_score = 0, local_tfidf = 0;
		int user_id;
	
		if(round_robin%3 == 0){
			//Geo sorted access	
			
			double user_dist = newP->dist;
			user_id = newP->id;
			local_geo_score = 1 - (user_dist/(double)MAXDIST);
			newP = gpos->getNextNN(x, y, incrStep);
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
			
				//Social RA
				int user_degree = spos->getUserDegree(user_id);
				local_soc_score = (double)user_degree/(double)MAXSC;
				
				//Keyword RA
				unordered_map<int,double>::iterator it = keywordMap.find(user_id);
				if(it != keywordMap.end()){
					local_tfidf = (*it).second;
				}
			}
			
			
			
			last_geo_score = local_geo_score;
		}
		else if(round_robin%3 == 1){	
		
			//Social sorted access
			my_pair user = *itSOC;
			local_soc_score = user.getScore();
			user_id = user.getId();
			++itSOC;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
				
				//Geo RA
				double userLocation[2];
				gpos->getLocation(user_id, userLocation);
				if(userLocation[0]!=-1000){ // is not null
					local_geo_score = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				}
				
				local_geo_score = 1 - (local_geo_score/(double)MAXDIST);
				
				//Keyword RA
				unordered_map<int,double>::iterator it = keywordMap.find(user_id);
				if(it != keywordMap.end()){
					local_tfidf = (*it).second;
				}
			}
			
			last_soc_score = local_soc_score;
		}
		else if(round_robin%3 == 2){	
			
			if(itKEY != keywordSet->end()){
				//Keyword sorted access
				my_pair user = *itKEY;
				user_id = user.getId();
				local_tfidf = user.getScore();
				++itKEY;
				
				ITseen = seen_users.find(user_id);
				if(ITseen == seen_users.end()){
					flag=true;
					seen_users.insert(user_id);
				
					//Geo RA
					double userLocation[2];
					gpos->getLocation(user_id, userLocation);
					if(userLocation[0]!=-1000){ // is not null
						local_geo_score = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
					}
					
					local_geo_score = 1 - (local_geo_score/(double)MAXDIST);
					
					//Social RA
					int user_degree = spos->getUserDegree(user_id);
					local_soc_score = (double)user_degree/(double)MAXSC;				
				}
				
				last_key_score = local_tfidf;
			}
			else{
				last_key_score = 0;
			}

		}

		if(flag==true)
			topk.insert(my_pair( user_id , local_soc_score + local_geo_score + local_tfidf));
		
		if(round_robin%3 == 2){
			//update threshold value
			threshold_value =  last_soc_score  + last_geo_score  + last_key_score  ;
			
			my_pair top1 =  *topk.begin();
			double score = top1.getScore();
			if(top1.getScore() > threshold_value){
				double userLocation[2];
				int id = top1.getId();
				// cout<<"adding to heap : user_id = "<<id<<" score = "<<score<<" threshold = "<<threshold_value<<endl;
				gpos->getLocation(id, userLocation);
				res_point* rp;
				if(userLocation[0]!=-1000){ // is not null
					double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
					rp = util.createResultPoint(id, userLocation[0], userLocation[1], dist);
				}
				topk.erase(top1);
				Group* newG = new Group(rp);
				newG->score = score;
				heap->push(new Group(newG));
				++heap_size;
			}
		}
		
		cout<<"RR: "<<round_robin<<" user_id = "<<user_id<<" social_score = "<<local_soc_score<<" | geo_score = "<<local_geo_score<<" | tfidf = "<<local_tfidf<<" | threshold = "<<threshold_value<<endl;
	
		++round_robin;
	}
	// cout<<"RR end = "<<round_robin<<endl;

	return heap;
}



class nra_user{

private:
	int id;
	double l_score;
	
	

public:
	nra_user(int id, double l_score, double h_score): id(id), l_score(l_score), h_score(h_score){seenG = false;seenS = false;seenK = false;}
	nra_user(const nra_user& other , double _l_score ) : id( other.id ), l_score( _l_score ), h_score( other.h_score), seenG( other.seenG ), seenS( other.seenS ), seenK( other.seenK ){  }
	nra_user(const nra_user& other ) : id( other.id ), l_score( other.l_score ), h_score( other.h_score ), seenG( other.seenG ), seenS( other.seenS ), seenK( other.seenK ){  }
	double h_score;
	
	
	bool seenG;
	bool seenS;
	bool seenK;
	int getId() const {return id;}
	double getLScore() const {return l_score;}
	double getHScore() const {return h_score;}
	void setLScore(double _l_score) {l_score = _l_score;}
	void setHScore(double _h_score) {h_score = _h_score;}
};


struct nra_comparator_descending{

	bool operator()(const nra_user  &x, const nra_user  &y){
		return y.getLScore() < x.getLScore();
	}
};




priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::SimpleNRA(double x, double y, unsigned int k, int incrStep, double w, vector<string> *terms){
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	
	unsigned int heap_size = 0;

	
	multiset<nra_user,nra_comparator_descending> buffer ; 
	//multiset<my_pair,pair_comparator_descending> topk ;
	
	map<int, multiset<nra_user,nra_comparator_descending>::iterator> map_of_iterator;
	
    map<int, multiset<nra_user,nra_comparator_descending>::iterator>::iterator moiIT;
	
	
	
	res_point* newP;
	multiset<my_pair,pair_comparator_descending>* degreeSet = spos->getDegreeSet();
	multiset<my_pair,pair_comparator_descending>::iterator itSOC = degreeSet->begin();
	// cout<<"degreeSet size = " <<degreeSet->size() << endl;
	
	multiset<my_pair,pair_comparator_descending>* keywordSet = new multiset<my_pair,pair_comparator_descending>();
	unordered_map<int, multiset< my_pair,pair_comparator_descending >::iterator> quick_search_map_key;

	double tfidf = 0;
	for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {

		vector<tf_pair>* posting_list = ivgen->getUsersWithTFfromVEC(*it);		
		for(std::vector<tf_pair>::iterator iter = posting_list->begin(); iter != posting_list->end(); ++iter) {
			
			tf_pair tf_user = *iter;
			int user_id = tf_user.getId();
			int term_frequency = tf_user.getTF();
			double idf = log10(DATASET_SIZE/(1 + posting_list->size()));
			double local_tfidf = term_frequency * idf;
			
			// cout<<" user ID = " << user_id <<" local tfidf = "<<local_tfidf <<" term_freq = "<<term_frequency<<" idf = "<< idf <<endl;
			auto it_id = quick_search_map_key.find(user_id);
				
			if(it_id!= quick_search_map_key.end()){ 
				multiset<my_pair,pair_comparator_descending>::iterator l_it = it_id->second;
				tfidf = local_tfidf + (*l_it).getScore();
				keywordSet->erase(l_it);
				keywordSet->insert(my_pair(user_id,tfidf));
				
			}
			else{
				quick_search_map_key.insert(make_pair(user_id,keywordSet->insert(my_pair(user_id,local_tfidf))));
			}
		}
		
	}
	
	// cout<<"keywordSet size = " <<keywordSet->size() << endl;
	// multiset<my_pair,pair_comparator_descending>::iterator asd;
	// for(asd = keywordSet->begin(); asd != keywordSet->end(); asd++){
		// my_pair user = *asd;
		// cout<<" user ID = " << user.getId() <<" local tfidf = "<<user.getScore()<<endl;
	// }
	
	
	multiset<my_pair,pair_comparator_descending>::iterator itKEY = keywordSet->begin();
	
	//social: use degreeSet for sequential access and spos->getuserDegree(x) for random access
	//geographical: use getUserLocation and calculate dist for random access. getkNN for sequential access
	
	set<int> seen_users;
	set<int>::iterator ITseen;
	int round_robin = 0;
	double local_geo_score = 0, local_soc_score = 0, local_tfidf = 0;
	newP = gpos->getNextNN(x, y, incrStep);
	int user_id_geo, user_id_soc, user_id_key;
	bool isG_end = false, isS_end = false, isK_end = false;

	while(heap_size != k){
		if(newP != NULL){
			//Geo sorted access
			double user_dist = newP->dist;
			user_id_geo = newP->id;
			local_geo_score = 1 - (user_dist/(double)MAXDIST);
			newP = gpos->getNextNN(x, y, incrStep);
		}
		else{
			isG_end = true; 
			local_geo_score = 0;
			user_id_geo = -1;
		}

		
		if(itSOC!=degreeSet->end()){
			//Social sorted access
			my_pair userSOC = *itSOC;
			local_soc_score = userSOC.getScore();
			user_id_soc = userSOC.getId();
			++itSOC;
		}
		else{
			isS_end = true;
			local_soc_score = 0;
			user_id_soc = -2;
		}
	
		if(itKEY != keywordSet->end()){
			//Keyword sorted access
			my_pair userKEY = *itKEY;
			user_id_key = userKEY.getId();
			local_tfidf = userKEY.getScore();
			++itKEY;
		}
		else{
			isK_end = true;
			local_tfidf = 0;
			user_id_key = -3;
		}
		
		if(itKEY == keywordSet->end() && itSOC==degreeSet->end() && newP == NULL){
			break;
		}
		
		
		if(user_id_geo == user_id_soc && user_id_soc != user_id_key){
		
			ITseen = seen_users.find(user_id_geo);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_geo);
				nra_user nra(user_id_geo, local_geo_score + local_soc_score, local_geo_score + local_soc_score + local_tfidf);
				nra.seenG = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_geo,insert_iterator));
			}
			else{ 							//update user's L score
				moiIT = map_of_iterator.find(user_id_geo);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_geo_score + local_soc_score);
				reinsertUSER.seenG = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_geo] = insert_iterator;
			}

			
			ITseen = seen_users.find(user_id_key);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_key);
				nra_user nra(user_id_key, local_tfidf, local_geo_score + local_soc_score + local_tfidf);
				nra.seenK = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_key,insert_iterator));

			}
			else{ 							//update user's L score
				moiIT = map_of_iterator.find(user_id_key);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_tfidf);
				reinsertUSER.seenK = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_key] = insert_iterator;
			
			}
			
			
		}
		else if(user_id_geo == user_id_key && user_id_key != user_id_soc){
			
			
			ITseen = seen_users.find(user_id_geo);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_geo);
				nra_user nra(user_id_geo, local_geo_score + local_tfidf, local_geo_score + local_soc_score + local_tfidf);
				nra.seenG = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_geo,insert_iterator));
			}
			else{ 							//update user's L score
				moiIT = map_of_iterator.find(user_id_geo);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_geo_score + local_tfidf);
				reinsertUSER.seenG = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_geo] = insert_iterator;
			}
			
			
			ITseen = seen_users.find(user_id_soc);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_soc);
				nra_user nra(user_id_soc, local_soc_score, local_geo_score + local_soc_score + local_tfidf);
				nra.seenS = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_soc,insert_iterator));

			}
			else{ 							//update user's score
				moiIT = map_of_iterator.find(user_id_soc);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_soc_score);
				reinsertUSER.seenS = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_soc] = insert_iterator;
			
			}
		}
		else if(user_id_soc == user_id_key && user_id_key != user_id_geo){
			
			ITseen = seen_users.find(user_id_key);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_key);
				nra_user nra(user_id_key, local_tfidf + local_soc_score, local_geo_score + local_soc_score + local_tfidf);
				nra.seenK = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_key,insert_iterator));

			}
			else{ 							//update user's L score
				moiIT = map_of_iterator.find(user_id_key);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_tfidf + local_soc_score);
				reinsertUSER.seenK = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_key] = insert_iterator;
			
			}
			
			
			ITseen = seen_users.find(user_id_geo);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_geo);
				nra_user nra(user_id_geo, local_geo_score, local_geo_score + local_soc_score + local_tfidf);
				nra.seenG = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_geo,insert_iterator));
			}
			else{ 							//update user's L score
				moiIT = map_of_iterator.find(user_id_geo);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_geo_score);
				reinsertUSER.seenG = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_geo] = insert_iterator;
			}
		}
		else if(user_id_soc == user_id_key && user_id_geo == user_id_key) {
			ITseen = seen_users.find(user_id_geo);
			if(ITseen == seen_users.end()){ //user not seen before
				seen_users.insert(user_id_geo);
				nra_user nra(user_id_geo, local_geo_score, local_geo_score + local_soc_score + local_tfidf);
				nra.seenG = true;
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
				map_of_iterator.insert(make_pair(user_id_geo,insert_iterator));
			}
			else{ 							//update user's L score
				moiIT = map_of_iterator.find(user_id_geo);
				nra_user foundUSER = *(moiIT->second);
				double old_score = foundUSER.getLScore();
				nra_user reinsertUSER(foundUSER, old_score + local_geo_score);
				reinsertUSER.seenG = true;
				buffer.erase(moiIT->second);
				multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
				map_of_iterator[user_id_geo] = insert_iterator;
			}
		}
		else{	//normal case
			
			if(!isG_end){
				ITseen = seen_users.find(user_id_geo);
				if(ITseen == seen_users.end()){ //user not seen before
					seen_users.insert(user_id_geo);
					nra_user nra(user_id_geo, local_geo_score, local_geo_score + local_soc_score + local_tfidf);
					nra.seenG = true;
					multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
					map_of_iterator.insert(make_pair(user_id_geo,insert_iterator));
				}
				else{ 							//update user's L score
					moiIT = map_of_iterator.find(user_id_geo);
					nra_user foundUSER = *(moiIT->second);
					double old_score = foundUSER.getLScore();
					nra_user reinsertUSER(foundUSER, old_score + local_geo_score);
					reinsertUSER.seenG = true;
					buffer.erase(moiIT->second);
					multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
					map_of_iterator[user_id_geo] = insert_iterator;
					// cout<<"updating user: "<<user_id_geo<<" score from "<<old_score<<" to "<<old_score + local_geo_score<<endl;
				}
			}
			
			if(!isK_end){
				ITseen = seen_users.find(user_id_key);
				if(ITseen == seen_users.end()){ //user not seen before
					seen_users.insert(user_id_key);
					nra_user nra(user_id_key, local_tfidf, local_geo_score + local_soc_score + local_tfidf);
					nra.seenK = true;
					multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
					map_of_iterator.insert(make_pair(user_id_key,insert_iterator));

				}
				else{ 							//update user's L score
					moiIT = map_of_iterator.find(user_id_key);
					nra_user foundUSER = *(moiIT->second);
					double old_score = foundUSER.getLScore();
					nra_user reinsertUSER(foundUSER, old_score + local_tfidf);
					reinsertUSER.seenK = true;
					buffer.erase(moiIT->second);
					multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
					map_of_iterator[user_id_key] = insert_iterator;
					// cout<<"updating user: "<<user_id_key<<" score from "<<old_score<<" to "<<old_score + local_tfidf<<endl;
				}
			}
		
			if(!isS_end){
				ITseen = seen_users.find(user_id_soc);
				if(ITseen == seen_users.end()){ //user not seen before
					seen_users.insert(user_id_soc);
					nra_user nra(user_id_soc, local_soc_score, local_geo_score + local_soc_score + local_tfidf);
					nra.seenS = true;
					multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(nra);
					map_of_iterator.insert(make_pair(user_id_soc,insert_iterator));
				}
				else{ 							//update user's L score
					moiIT = map_of_iterator.find(user_id_soc);
					nra_user foundUSER = *(moiIT->second);
					double old_score = foundUSER.getLScore();
					nra_user reinsertUSER(foundUSER, old_score + local_soc_score);
					reinsertUSER.seenS = true;
					buffer.erase(moiIT->second);
					multiset<nra_user,nra_comparator_descending>::iterator insert_iterator = buffer.insert(reinsertUSER);
					map_of_iterator[user_id_soc] = insert_iterator;	
					// cout<<"updating user: "<<user_id_soc<<" score from "<<old_score<<" to "<<old_score + local_soc_score<<endl;			
				}
			}
		}
		multiset<nra_user,nra_comparator_descending>::iterator iter;
		//update all buffer user's H score
		for(iter = buffer.begin(); iter != buffer.end(); ++iter){
			const nra_user &usertemp=*iter;
			nra_user &user = const_cast<nra_user&>(usertemp);
			double l_score = user.getLScore();
			
			if(!user.seenG && user.seenS && user.seenK){
				// user.setHScore(l_score + local_geo_score);
				user.h_score = l_score + local_geo_score;
			}
			else if(user.seenG && !user.seenS && user.seenK){
				// user.setHScore(l_score + local_soc_score);
				user.h_score =l_score + local_soc_score;
			}
			else if(user.seenG && user.seenS && !user.seenK){
				// user.setHScore(l_score + local_tfidf);
				user.h_score = l_score + local_tfidf;
			}
			else if(!user.seenG && !user.seenS && user.seenK){
				// user.setHScore(l_score + local_geo_score + local_soc_score);
				user.h_score = l_score + local_geo_score + local_soc_score;
			}
			else if(user.seenG && !user.seenS && !user.seenK){
				// user.setHScore(l_score + local_soc_score + local_tfidf);
				user.h_score = l_score + local_soc_score + local_tfidf;
			}
			else if(!user.seenG && user.seenS && !user.seenK){
				// user.setHScore(l_score + local_tfidf + local_geo_score);
				user.h_score = l_score + local_tfidf + local_geo_score;
			}

		}
		
		// for(iter = buffer.begin(); iter != buffer.end(); ++iter){
			// nra_user user = *iter;
			// if(user.getId() == 32158){
				// cout<<"\t what the fuck user_id = "<<user.getId()<<" H score is now = "<<user.getHScore()<<" social_score = "<<local_soc_score<<" | geo_score = "<<local_geo_score<<" | tfidf = "<<local_tfidf<<endl;
			// }
		// }

		iter = buffer.begin();
		nra_user top_user = *iter;
		// int top_UID = top_user.getId();
		double top_score = top_user.getLScore();
		double highest_score = 0;
		// int high_UID = 0;
		++iter;
		for(; iter != buffer.end(); ++iter){
			nra_user user = *iter;
			double score = user.getHScore();
			
			// if(user_id_soc == 6383){
				// cout<<"\t Buffer user_id = "<<user.getId()<<" top score is "<<top_score<<" H score = "<<h_score<<endl;
			// }
			
			if(top_score >= score) {
				if(score > highest_score){
					highest_score = score;
					// high_UID = user.getId();
				}
				continue;
			}
			else{
				highest_score = score;
				// high_UID = user.getId();
				break;
			}
		}
		
		if(iter == buffer.end()){	// found user with l score >= all h scores
		
			double userLocation[2];
			int id = top_user.getId();
			buffer.erase(buffer.begin());
			// cout<<"adding to heap : user_id = "<<id<<endl;
			gpos->getLocation(id, userLocation);
			res_point* rp;
			if(userLocation[0]!=-1000){ // is not null
				double dist = util.computeMinimumDistance(x, y, userLocation[0], userLocation[1]);
				rp = util.createResultPoint(id, userLocation[0], userLocation[1], dist);
			}
		
			Group* newG = new Group(rp);
			newG->score = top_score;
			heap->push(new Group(newG));
			++heap_size;
		}
		
		// cout<<"RR: "<<round_robin<<" user_id_geo = "<<user_id_geo<<" user_id_soc = "<<user_id_soc<<" user_id_key = "<<user_id_key<<" social_score = "<<local_soc_score<<" | geo_score = "<<local_geo_score<<" | tfidf = "<<local_tfidf<<" size_of_buffer "<<buffer.size()<< " topUID = " <<top_UID<<" L-Score = "<< top_score <<" highUID = "<<high_UID<<" highest Score ="<<highest_score <<endl;
		
		
		++round_robin;
	}

	return heap;
}


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::GridBasedTA(double x, double y, unsigned int k, double w, double _radius, vector<string> *terms){
	
	struct timeval startGEO, endGEO, startSOC, endSOC, startKEY, endKEY;
	// cout <<"radius = "<<_radius<<" | "<<_radius*EARTH_CIRCUMFERENCE/360<<"km"<<endl;
	
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	
	
	set<res_point*, res_point_ascending_dist>* usersInValidRange;
	vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* keywordVEC =  new vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>();
	
	gettimeofday(&startGEO, NULL);
	
	//get keywords and all the users in range
	for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
		keywordVEC->push_back(new vector<multiset<tf_pair, pair_descending_frequency>*>());
	}
	
	usersInValidRange = gpos->getRangeWithIDX(x, y, _radius, terms, keywordVEC);
	usersChecked = usersInValidRange->size();
	
	//unorderedmap for random access checking in GEO
	unordered_map <int, res_point *> res_map;
	set<res_point*, res_point_ascending_dist>::iterator itGEO = usersInValidRange->begin();
	
	gettimeofday(&endGEO, NULL);
	cout<<"Total Time for getRangeWithIDX= "<<print_time(startGEO, endGEO)/1000<<" ms"<<endl;
	
	
	gettimeofday(&startSOC, NULL);
	
	int* friends;
	unsigned int friendsSize;
	for(auto iter_range = usersInValidRange->begin(); iter_range != usersInValidRange->end(); ++iter_range){
		res_point* newP = *iter_range;
		res_map[newP->id] = newP;
	}
	unordered_map<int,res_point *>::iterator got;
	cout<<"Number of users in range"<<usersInValidRange->size()<<endl;
	
	unordered_map<int, Group*> soc_map;
	multiset<my_pair,pair_comparator_descending> soc_set ;
	Group* newG;
	unsigned int max_social= 0;
	//unorderedmap for random access checking in SOC
	for(auto iter_range = usersInValidRange->begin(); iter_range != usersInValidRange->end(); ++iter_range){
		res_point* newP = *iter_range;
		//cout << "i = " << i << "\t id = " << newP->id << endl;
		int soc_id = newP->id;
		spos->getFriends(soc_id, friends, friendsSize);
		
		
		
		Group* tempG = (Group*)malloc(sizeof(Group));
		newG = new(tempG) Group(newP); 

		for(unsigned int v = 0; v < friendsSize; v++){
			int f_id = friends[v];
			got = res_map.find(f_id);
			if(got != res_map.end()){
			
				//put friends in group for output.
				newG->addFriend(got->second);
				
			}
		}
		unsigned int size_of_group = newG->size();
		
		if(size_of_group > max_social){
			max_social = size_of_group;
		}
		
		soc_set.insert(my_pair(soc_id,(double)size_of_group));
		soc_map.insert(make_pair(soc_id,newG));
	}
	multiset<my_pair,pair_comparator_descending>::iterator itSOC = soc_set.begin();
	
	gettimeofday(&endSOC, NULL);
	cout<<"Total Time for social score calculation= "<<print_time(startSOC, endSOC)/1000<<" ms"<<endl;
	// cout<<"Local Max social = "<<max_social<<endl;; 

	gettimeofday(&startKEY, NULL);
	
	
	// cout<<"keyword vector size (represents the number of terms): "<<keywordVEC->size()<<endl;
	// for(auto it = keywordVEC->begin(); it != keywordVEC->end(); ++it) {
		// vector<multiset<tf_pair, pair_descending_frequency>*>* posting_lists_vector = *it;
		// cout<<"\t size of vector of multisets (represents number of cells): "<< posting_lists_vector->size()<<endl;
		// for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			// multiset<tf_pair, pair_descending_frequency>* pl = *iter;
			// cout << "\t size of set of users containing term: "<< pl->size() <<endl;
			// for(auto it_s = pl->begin(); it_s != pl->end(); ++it_s) {
				// tf_pair tf_user = *it_s;
				// cout<<"\t\t set contains users: "<<tf_user.getId()<<endl;
			// }
		// }
		
	// }
	
	//unordered map for keywords
	multiset<my_pair,pair_comparator_descending>* key_set = new multiset<my_pair,pair_comparator_descending>();
	unordered_map<int,double> key_map;
	unordered_map<int, multiset< my_pair,pair_comparator_descending >::iterator> quick_search_map_key;
	for(auto it = keywordVEC->begin(); it != keywordVEC->end(); ++it) {
		unsigned int size_of_pl = 0;
		vector<multiset<tf_pair, pair_descending_frequency>*>* posting_lists_vector = *it;
		for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			size_of_pl += (*iter)->size();
		}
		for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			multiset<tf_pair, pair_descending_frequency>* pl = *iter;
			double tfidf = 0;
			for(auto set_iter = pl->begin(); set_iter != pl->end();++set_iter){
				tf_pair tf_user = *set_iter;
				int u_id = tf_user.getId();
				int term_frequency = tf_user.getTF();
				double idf = log10(DATASET_SIZE/(double)(1 + size_of_pl));
				double local_tfidf = log10(1+ term_frequency) * idf;
			
				// cout<<" user ID = " << u_id <<" local tfidf = "<<local_tfidf <<" term_freq = "<<term_frequency<<" idf = "<< idf <<endl;
				auto it_id = quick_search_map_key.find(u_id);
				
				if(it_id!= quick_search_map_key.end()){ 
					multiset<my_pair,pair_comparator_descending>::iterator l_it = it_id->second;
					tfidf = local_tfidf + (*l_it).getScore();
					key_map[u_id]=tfidf;
					key_set->erase(l_it);
					key_set->insert(my_pair(u_id,tfidf));
					
				}
				else{
					key_map[u_id]=local_tfidf;
					quick_search_map_key.insert(make_pair(u_id,key_set->insert(my_pair(u_id,local_tfidf))));
				}
			}
		}

	}
	multiset<my_pair,pair_comparator_descending>::iterator itKEY = key_set->begin();
	
	
	gettimeofday(&endKEY, NULL);
	cout<<"Total Time for keyword score calculation= "<<print_time(startKEY, endKEY)/1000<<" ms"<<endl;
	
	//initiate variables
	unsigned int heap_size = 0;
	double threshold_value = -1;
	double last_soc_score  = 0;
	double last_geo_score  = 0;
	double last_key_score  = 0;
	
	multiset<my_pair,pair_comparator_descending> topk ;
	
	
	//execute SIMPLE TA in round robin.
	
	
	
	//social: use degreeSet for sequential access and spos->getuserDegree(x) for random access
	//geographical: use getUserLocation and calculate dist for random access. getkNN for sequential access
	
	set<int> seen_users;
	set<int>::iterator ITseen;
	int round_robin = 0;
	res_point* newP = *itGEO;
	
	while(heap_size != k && newP != NULL && itSOC!=soc_set.end()){
		bool flag = false;
		double local_geo_score = 0, local_soc_score = 0, local_tfidf = 0;
		int user_id;
	
		if(round_robin%3 == 0){
			//Geo sorted access	
			user_id = newP->id;
			local_geo_score = 1 - ((double)newP->dist/(double)_radius);
			// cout <<"geo sort access - newP id: "<<newP->id<<" dist:  "<<newP->dist<<" | "<<newP->dist*EARTH_CIRCUMFERENCE/360 <<" | score = "<<local_geo_score<<endl;
			
			++itGEO;
			newP = *itGEO;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
			
				//Social RA
				auto it_soc_map = soc_map.find(user_id);
				if(it_soc_map != soc_map.end())
					local_soc_score = (double)((it_soc_map->second)->size())/(double)max_social;
				else
					local_soc_score = 0;
				
				//Keyword RA
				unordered_map<int,double>::iterator it = key_map.find(user_id);
				if(it != key_map.end()){
					local_tfidf = (*it).second;
				}
				else{
					local_tfidf = 0;
				}
			}
			
			
			
			last_geo_score = local_geo_score;
		}
		else if(round_robin%3 == 1){	
		
			//Social sorted access
			my_pair user = *itSOC;
			local_soc_score = (double)user.getScore()/(double)max_social;
			user_id = user.getId();
			++itSOC;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
				
				//Geo RA
				res_point* localP = res_map[user_id];
				// cout <<"newPdist = "<<localP->dist<<" | "<<localP->dist*EARTH_CIRCUMFERENCE/360<<endl;
				local_geo_score = 1 - ((double)localP->dist/(double)_radius);
				
				//Keyword RA
				unordered_map<int,double>::iterator it = key_map.find(user_id);
				if(it != key_map.end()){
					local_tfidf = (*it).second;
				}
				else{
					local_tfidf = 0;
				}
			}
			
			last_soc_score = local_soc_score;
		}
		else if(round_robin%3 == 2){	
			while(true){
				if(itKEY != key_set->end()){
					//Keyword sorted access
					my_pair user = *itKEY;
					user_id = user.getId();
					local_tfidf = user.getScore();
					++itKEY;
					
					ITseen = seen_users.find(user_id);
					if(ITseen == seen_users.end()){
						
						//Geo RA
						auto it_res_map = res_map.find(user_id);
						if(it_res_map != res_map.end()){
							res_point* localP = res_map[user_id];
							local_geo_score = 1 - ((double)localP->dist/(double)_radius);
						}
						else{
							break;
						}
						
						//Social RA
						auto it_soc_map = soc_map.find(user_id);
						if(it_soc_map != soc_map.end())
							local_soc_score =(double)(it_soc_map->second)->size()/(double)max_social;
						else
							local_soc_score = 0;
							
							
						flag=true;
						seen_users.insert(user_id);
							
					}
				
					last_key_score = local_tfidf;
					break;
				}
				else{
					//SORTED ACCESS INTO GEO
					user_id = newP->id;
					local_geo_score = 1 - ((double)newP->dist/(double)_radius);
					++itGEO;
					newP = *itGEO;
					
					ITseen = seen_users.find(user_id);
					if(ITseen == seen_users.end()){
						flag=true;
						seen_users.insert(user_id);
					
						//Social RA
						auto it_soc_map = soc_map.find(user_id);
						if(it_soc_map != soc_map.end())
							local_soc_score = (double)(it_soc_map->second)->size()/(double)max_social;
						else
							local_soc_score = 0;		
					}
					
					local_tfidf = 0;
					last_key_score = 0;
					break;
				}
			}
		}
		
		if(flag==true)
			topk.insert(my_pair( user_id , local_soc_score + local_geo_score + local_tfidf));
		
		if(round_robin%3 == 2){
			// update threshold value
			threshold_value =  last_soc_score  + last_geo_score  + last_key_score  ;
			
			my_pair top1 =  *topk.begin();
			double score = top1.getScore();
			if(top1.getScore() > threshold_value){
				int id = top1.getId();
				//cout<<"adding to heap : user_id = "<<id<<" score = "<<score<<" threshold = "<<threshold_value<<endl;
				topk.erase(top1);
				Group* topG = new Group(soc_map[id]);
				topG->score = score;
				heap->push(topG);
				++heap_size;
			}
		}
		
		// cout<<"RR: "<<round_robin<<" user_id = "<<user_id<<" geo_score = "<<local_geo_score<<" social_score = "<<local_soc_score<<" tfidf = "<<local_tfidf<<" | threshold = "<<threshold_value<<endl;
	
		++round_robin;
	}
	cout<<"RR end = "<<round_robin<<endl;

	return heap;
}




priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::GridBasedNRA(double x, double y, unsigned int k, double w, double r, vector<string> *terms){
		
	Group* newG;
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	double best_score = -1;
	unsigned int heap_size = 0;
	vector<res_point*>* usersInValidRange;
	usersChecked =0;
	threshold =0;

	double radius = w*r/(1-w);


	usersInValidRange = gpos->getRange(x, y, radius);
	threshold = radius;

	usersChecked=usersInValidRange->size();
	res_point *tmp;
	int* friends;
	unsigned int friendsSize;
	for(int i = 0; i < (int)usersInValidRange->size(); i++){
	
		
	}

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

		for (unsigned int i = heap_size; i < k ; i++){
			nearestP = kNN->at(i);
			Group* gp = new Group(nearestP);
			gp->score=0;
			heap->push(gp);
		}
	}

	return heap;

}


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::testBLOOM(double x, double y, int k, double w, double r, vector<string> *terms){
		
	Group* newG;
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	double best_score = -1;
	int heap_size = 0;
	vector<res_point*>* usersInValidRange;
	usersChecked =0;
	threshold =0;
	unordered_map <int, res_point *> umap;
	double radius = w*r/(1-w);

	usersInValidRange = gpos->getRange(x, y, radius);
	threshold = radius;

	usersChecked=usersInValidRange->size();
	int* friends;
	unsigned int friendsSize;
	for(int i = 0; i < (int)usersInValidRange->size(); i++){
		res_point* newP = (*usersInValidRange)[i];
		umap[newP->id] = newP;
	}
	unordered_map<int,res_point *>::iterator got;
	
	for(int i = 0; i < (int)usersInValidRange->size(); i++){
		res_point* newP = (*usersInValidRange)[i];
		//cout << "i = " << i << "\t id = " << newP->id << endl;
		spos->getFriends(newP->id, friends, friendsSize);
		newG = new Group(newP);

		for(unsigned int v = 0; v < friendsSize; v++){
			int f_id = friends[v];
			got = umap.find(f_id);
			if(got != umap.end()){
				newG->addFriend(got->second);
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

priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::GridBasedTA_bloom(double x, double y, unsigned int k, double w, double _radius, vector<string> *terms){
	
	struct timeval startGEO, endGEO, startSOC, endSOC, startKEY, endKEY;
	//double sumBound=0, sumScoring =0, sumGF=0, sumA=0;
	
	// cout <<"radius = "<<_radius<<" | "<<_radius*EARTH_CIRCUMFERENCE/360<<"km"<<endl;
	
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	
	set<res_point*, res_point_ascending_dist>* usersInValidRange;
	vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* keywordVEC =  new vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>();
	
	gettimeofday(&startGEO, NULL);
	
	//get keywords and all the users in range
	for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
		keywordVEC->push_back(new vector<multiset<tf_pair, pair_descending_frequency>*>());
	}
	
	usersInValidRange = gpos->getRangeWithIDX(x, y, _radius, terms, keywordVEC);
	usersChecked =usersInValidRange->size();
	
	//unorderedmap for random access checking in GEO
	unordered_map <int, res_point *> res_map;
	set<res_point*, res_point_ascending_dist>::iterator itGEO = usersInValidRange->begin();
	
	gettimeofday(&endGEO, NULL);
	cout<<"Total Time for getRangeWithIDX= "<<print_time(startGEO, endGEO)/1000<<" ms"<<endl;
	
	
	gettimeofday(&startSOC, NULL);
	
	counting_bloom_t *range_bloom = (counting_bloom_t *) gpos->getRangeBloom(x,y,_radius);
	
	int* friends;
	unsigned int friendsSize;
	for(auto iter_range = usersInValidRange->begin(); iter_range != usersInValidRange->end(); ++iter_range){
		res_point* newP = *iter_range;
		res_map[newP->id] = newP;
	}
	unordered_map<int,res_point *>::iterator got;
	
	
	unordered_map<int, Group*> soc_map;
	multiset<my_pair,pair_comparator_descending> soc_set ;
	Group* newG;
	unsigned int max_social= 0;
	
	cout<<"Number of users in range"<<usersInValidRange->size()<<endl;
	//unorderedmap for random access checking in SOC
	for(auto iter_range = usersInValidRange->begin(); iter_range != usersInValidRange->end(); ++iter_range){
		res_point* newP = *iter_range;
		//cout << "i = " << i << "\t id = " << newP->id << endl;
		int soc_id = newP->id;
		spos->getFriends(soc_id, friends, friendsSize);
		
		Group* tempG = (Group *)malloc(sizeof(Group));
		newG = new(tempG) Group(newP); 
	
		for(unsigned int v = 0; v < friendsSize; v++){
			int f_id = friends[v];
			ostringstream ss;
			ss << f_id;
			string f_id_s = ss.str();
			if(counting_bloom_check(range_bloom, f_id_s.c_str(), strlen(f_id_s.c_str()))){
				//put friends in group for output.
				auto hit = res_map.find(f_id);
				if(hit != res_map.end()){
					res_point* localP = res_map[f_id];
					newG->addFriend(localP);
				}
			}
		}
		unsigned int size_of_group = newG->size();
		// cout<<"Size of friend group = "<<size_of_group<<endl;
		if(size_of_group > max_social){
			max_social = size_of_group;
		}
		
		soc_set.insert(my_pair(soc_id,(double)size_of_group));
		soc_map.insert(make_pair(soc_id,newG));
	}
	multiset<my_pair,pair_comparator_descending>::iterator itSOC = soc_set.begin();
	
	gettimeofday(&endSOC, NULL);
	cout<<"Total Time for social score calculation= "<<print_time(startSOC, endSOC)/1000<<" ms"<<endl;
	// cout<<"Local Max social = "<<max_social<<endl;; 

	gettimeofday(&startKEY, NULL);
	
	
	// cout<<"keyword vector size (represents the number of terms): "<<keywordVEC->size()<<endl;
	// for(auto it = keywordVEC->begin(); it != keywordVEC->end(); ++it) {
		// vector<multiset<tf_pair, pair_descending_frequency>*>* posting_lists_vector = *it;
		// cout<<"\t size of vector of multisets (represents number of cells): "<< posting_lists_vector->size()<<endl;
		// for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			// multiset<tf_pair, pair_descending_frequency>* pl = *iter;
			// cout << "\t size of set of users containing term: "<< pl->size() <<endl;
			// for(auto it_s = pl->begin(); it_s != pl->end(); ++it_s) {
				// tf_pair tf_user = *it_s;
				// cout<<"\t\t set contains users: "<<tf_user.getId()<<endl;
			// }
		// }
		
	// }
	
	//unordered map for keywords
	multiset<my_pair,pair_comparator_descending>* key_set = new multiset<my_pair,pair_comparator_descending>();
	unordered_map<int,double> key_map;
	unordered_map<int, multiset< my_pair,pair_comparator_descending >::iterator> quick_search_map_key;
	for(auto it = keywordVEC->begin(); it != keywordVEC->end(); ++it) {
		unsigned int size_of_pl = 0;
		vector<multiset<tf_pair, pair_descending_frequency>*>* posting_lists_vector = *it;
		for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			size_of_pl += (*iter)->size();
		}
		for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			multiset<tf_pair, pair_descending_frequency>* pl = *iter;
			double tfidf = 0;
			for(auto set_iter = pl->begin(); set_iter != pl->end();++set_iter){
				tf_pair tf_user = *set_iter;
				int u_id = tf_user.getId();
				int term_frequency = tf_user.getTF();
				double idf = log10(DATASET_SIZE/(double)(1 + size_of_pl));
				double local_tfidf = log10(1+ term_frequency) * idf;
			
				// cout<<" user ID = " << u_id <<" local tfidf = "<<local_tfidf <<" term_freq = "<<term_frequency<<" idf = "<< idf <<endl;
				auto it_id = quick_search_map_key.find(u_id);
				
				if(it_id!= quick_search_map_key.end()){ 
					multiset<my_pair,pair_comparator_descending>::iterator l_it = it_id->second;
					tfidf = local_tfidf + (*l_it).getScore();
					key_map[u_id]=tfidf;
					key_set->erase(l_it);
					key_set->insert(my_pair(u_id,tfidf));
					
				}
				else{
					key_map[u_id]=local_tfidf;
					quick_search_map_key.insert(make_pair(u_id,key_set->insert(my_pair(u_id,local_tfidf))));
				}
			}
		}

	}
	multiset<my_pair,pair_comparator_descending>::iterator itKEY = key_set->begin();
	
	
	gettimeofday(&endKEY, NULL);
	cout<<"Total Time for keyword score calculation= "<<print_time(startKEY, endKEY)/1000<<" ms"<<endl;
	
	//initiate variables
	unsigned int heap_size = 0;
	double threshold_value = -1;
	double last_soc_score  = 0;
	double last_geo_score  = 0;
	double last_key_score  = 0;
	
	multiset<my_pair,pair_comparator_descending> topk ;
	
	
	//execute SIMPLE TA in round robin.
	
	
	
	//social: use degreeSet for sequential access and spos->getuserDegree(x) for random access
	//geographical: use getUserLocation and calculate dist for random access. getkNN for sequential access
	
	set<int> seen_users;
	set<int>::iterator ITseen;
	int round_robin = 0;
	res_point* newP = *itGEO;
	
	while(heap_size != k && newP != NULL && itSOC!=soc_set.end()){
		bool flag = false;
		double local_geo_score = 0, local_soc_score = 0, local_tfidf = 0;
		int user_id;
	
		if(round_robin%3 == 0){
			//Geo sorted access	
			user_id = newP->id;
			local_geo_score = 1 - ((double)newP->dist/(double)_radius);
			// cout <<"geo sort access - newP id: "<<newP->id<<" dist:  "<<newP->dist<<" | "<<newP->dist*EARTH_CIRCUMFERENCE/360 <<" | score = "<<local_geo_score<<endl;
			
			++itGEO;
			newP = *itGEO;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
			
				//Social RA
				auto it_soc_map = soc_map.find(user_id);
				if(it_soc_map != soc_map.end())
					local_soc_score = (double)((it_soc_map->second)->size())/(double)max_social;
				else
					local_soc_score = 0;
				
				//Keyword RA
				unordered_map<int,double>::iterator it = key_map.find(user_id);
				if(it != key_map.end()){
					local_tfidf = (*it).second;
				}
				else{
					local_tfidf = 0;
				}
			}
			
			
			
			last_geo_score = local_geo_score;
		}
		else if(round_robin%3 == 1){	
		
			//Social sorted access
			my_pair user = *itSOC;
			local_soc_score = (double)user.getScore()/(double)max_social;
			user_id = user.getId();
			++itSOC;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
				
				//Geo RA
				res_point* localP = res_map[user_id];
				// cout <<"newPdist = "<<localP->dist<<" | "<<localP->dist*EARTH_CIRCUMFERENCE/360<<endl;
				local_geo_score = 1 - ((double)localP->dist/(double)_radius);
				
				//Keyword RA
				unordered_map<int,double>::iterator it = key_map.find(user_id);
				if(it != key_map.end()){
					local_tfidf = (*it).second;
				}
				else{
					local_tfidf = 0;
				}
			}
			
			last_soc_score = local_soc_score;
		}
		else if(round_robin%3 == 2){	
			while(true){
				if(itKEY != key_set->end()){
					//Keyword sorted access
					my_pair user = *itKEY;
					user_id = user.getId();
					local_tfidf = user.getScore();
					++itKEY;
					
					ITseen = seen_users.find(user_id);
					if(ITseen == seen_users.end()){
						
						//Geo RA
						auto it_res_map = res_map.find(user_id);
						if(it_res_map != res_map.end()){
							res_point* localP = res_map[user_id];
							local_geo_score = 1 - ((double)localP->dist/(double)_radius);
						}
						else{
							break;
						}
						
						//Social RA
						auto it_soc_map = soc_map.find(user_id);
						if(it_soc_map != soc_map.end())
							local_soc_score =(double)(it_soc_map->second)->size()/(double)max_social;
						else
							local_soc_score = 0;
							
							
						flag=true;
						seen_users.insert(user_id);
							
					}
				
					last_key_score = local_tfidf;
					break;
				}
				else{
					//SORTED ACCESS INTO GEO
					user_id = newP->id;
					local_geo_score = 1 - ((double)newP->dist/(double)_radius);
					++itGEO;
					newP = *itGEO;
					
					ITseen = seen_users.find(user_id);
					if(ITseen == seen_users.end()){
						flag=true;
						seen_users.insert(user_id);
					
						//Social RA
						auto it_soc_map = soc_map.find(user_id);
						if(it_soc_map != soc_map.end())
							local_soc_score = (double)(it_soc_map->second)->size()/(double)max_social;
						else
							local_soc_score = 0;		
					}
					
					local_tfidf = 0;
					last_key_score = 0;
					break;
				}
			}
		}
		
		if(flag==true)
			topk.insert(my_pair( user_id , local_soc_score + local_geo_score + local_tfidf));
		
		if(round_robin%3 == 2){
			// update threshold value
			threshold_value =  last_soc_score  + last_geo_score  + last_key_score  ;
			
			my_pair top1 =  *topk.begin();
			double score = top1.getScore();
			if(top1.getScore() > threshold_value){
				int id = top1.getId();
				//cout<<"adding to heap : user_id = "<<id<<" score = "<<score<<" threshold = "<<threshold_value<<endl;
				topk.erase(top1);
				Group* topG = new Group(soc_map[id]);
				topG->score = score;
				heap->push(topG);
				++heap_size;
			}
		}
		
		// cout<<"RR: "<<round_robin<<" user_id = "<<user_id<<" geo_score = "<<local_geo_score<<" social_score = "<<local_soc_score<<" tfidf = "<<local_tfidf<<" | threshold = "<<threshold_value<<endl;
	
		++round_robin;
	}
	// cout<<"RR end = "<<round_robin<<endl;

	return heap;
}

priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::GridBasedTA_naive(double x, double y, unsigned int k, double w, double _radius, vector<string> *terms){
	struct timeval startGEO, endGEO, startSOC, endSOC, startKEY, endKEY;
	//double sumBound=0, sumScoring =0, sumGF=0, sumA=0;
	
	// cout <<"radius = "<<_radius<<" | "<<_radius*EARTH_CIRCUMFERENCE/360<<"km"<<endl;
	
	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	
	set<res_point*, res_point_ascending_dist>* usersInValidRange;
	vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* keywordVEC =  new vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>();
	
	gettimeofday(&startGEO, NULL);
	
	//get keywords and all the users in range
	for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
		keywordVEC->push_back(new vector<multiset<tf_pair, pair_descending_frequency>*>());
	}
	
	usersInValidRange = gpos->getRangeWithIDX(x, y, _radius, terms, keywordVEC);
	usersChecked =usersInValidRange->size();
	
	//unorderedmap for random access checking in GEO
	unordered_map <int, res_point *> res_map;
	set<res_point*, res_point_ascending_dist>::iterator itGEO = usersInValidRange->begin();
	
	gettimeofday(&endGEO, NULL);
	cout<<"Total Time for getRangeWithIDX= "<<print_time(startGEO, endGEO)/1000<<" ms"<<endl;
	
	
	gettimeofday(&startSOC, NULL);
	
	// counting_bloom_t *range_bloom = (counting_bloom_t *) gpos->getRangeBloom(x,y,_radius);
	
	int* friends;
	unsigned int friendsSize;
	for(auto iter_range = usersInValidRange->begin(); iter_range != usersInValidRange->end(); ++iter_range){
		res_point* newP = *iter_range;
		res_map[newP->id] = newP;
	}
	unordered_map<int,res_point *>::iterator got;
	
	
	unordered_map<int, Group*> soc_map;
	multiset<my_pair,pair_comparator_descending> soc_set ;
	Group* newG;
	unsigned int max_social= 0;
	
	cout<<"Number of users in range"<<usersInValidRange->size()<<endl;
	//unorderedmap for random access checking in SOC
	for(auto iter_range = usersInValidRange->begin(); iter_range != usersInValidRange->end(); ++iter_range){
		res_point* newP = *iter_range;
		//cout << "i = " << i << "\t id = " << newP->id << endl;
		int soc_id = newP->id;
		spos->getFriends(soc_id, friends, friendsSize);
		
		Group* tempG = (Group *)malloc(sizeof(Group));
		newG = new(tempG) Group(newP); 

		for(auto iter_f = usersInValidRange->begin(); iter_f != usersInValidRange->end(); ++iter_f){
			res_point* tmp = *iter_f;

			if(util.binarySearch(friends, 0, friendsSize, tmp->id)){
				newG->addFriend(tmp);
			}
		}
		unsigned int size_of_group = newG->size();
		// cout<<"Size of friend group = "<<size_of_group<<endl;
		if(size_of_group > max_social){
			max_social = size_of_group;
		}
		
		soc_set.insert(my_pair(soc_id,(double)size_of_group));
		soc_map.insert(make_pair(soc_id,newG));
	}
	multiset<my_pair,pair_comparator_descending>::iterator itSOC = soc_set.begin();
	
	gettimeofday(&endSOC, NULL);
	cout<<"Total Time for social score calculation= "<<print_time(startSOC, endSOC)/1000<<" ms"<<endl;
	// cout<<"Local Max social = "<<max_social<<endl;; 

	gettimeofday(&startKEY, NULL);
	
	
	// cout<<"keyword vector size (represents the number of terms): "<<keywordVEC->size()<<endl;
	// for(auto it = keywordVEC->begin(); it != keywordVEC->end(); ++it) {
		// vector<multiset<tf_pair, pair_descending_frequency>*>* posting_lists_vector = *it;
		// cout<<"\t size of vector of multisets (represents number of cells): "<< posting_lists_vector->size()<<endl;
		// for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			// multiset<tf_pair, pair_descending_frequency>* pl = *iter;
			// cout << "\t size of set of users containing term: "<< pl->size() <<endl;
			// for(auto it_s = pl->begin(); it_s != pl->end(); ++it_s) {
				// tf_pair tf_user = *it_s;
				// cout<<"\t\t set contains users: "<<tf_user.getId()<<endl;
			// }
		// }
		
	// }
	
	//unordered map for keywords
	multiset<my_pair,pair_comparator_descending>* key_set = new multiset<my_pair,pair_comparator_descending>();
	unordered_map<int,double> key_map;
	unordered_map<int, multiset< my_pair,pair_comparator_descending >::iterator> quick_search_map_key;
	for(auto it = keywordVEC->begin(); it != keywordVEC->end(); ++it) {
		unsigned int size_of_pl = 0;
		vector<multiset<tf_pair, pair_descending_frequency>*>* posting_lists_vector = *it;
		for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			size_of_pl += (*iter)->size();
		}
		for(auto iter = posting_lists_vector->begin(); iter != posting_lists_vector->end(); ++iter) {
			multiset<tf_pair, pair_descending_frequency>* pl = *iter;
			double tfidf = 0;
			for(auto set_iter = pl->begin(); set_iter != pl->end();++set_iter){
				tf_pair tf_user = *set_iter;
				int u_id = tf_user.getId();
				int term_frequency = tf_user.getTF();
				double idf = log10(DATASET_SIZE/(double)(1 + size_of_pl));
				double local_tfidf = log10(1+ term_frequency) * idf;
			
				// cout<<" user ID = " << u_id <<" local tfidf = "<<local_tfidf <<" term_freq = "<<term_frequency<<" idf = "<< idf <<endl;
				auto it_id = quick_search_map_key.find(u_id);
				
				if(it_id!= quick_search_map_key.end()){ 
					multiset<my_pair,pair_comparator_descending>::iterator l_it = it_id->second;
					tfidf = local_tfidf + (*l_it).getScore();
					key_map[u_id]=tfidf;
					key_set->erase(l_it);
					key_set->insert(my_pair(u_id,tfidf));
					
				}
				else{
					key_map[u_id]=local_tfidf;
					quick_search_map_key.insert(make_pair(u_id,key_set->insert(my_pair(u_id,local_tfidf))));
				}
			}
		}

	}
	multiset<my_pair,pair_comparator_descending>::iterator itKEY = key_set->begin();
	
	
	gettimeofday(&endKEY, NULL);
	cout<<"Total Time for keyword score calculation= "<<print_time(startKEY, endKEY)/1000<<" ms"<<endl;
	
	//initiate variables
	unsigned int heap_size = 0;
	double threshold_value = -1;
	double last_soc_score  = 0;
	double last_geo_score  = 0;
	double last_key_score  = 0;
	
	multiset<my_pair,pair_comparator_descending> topk ;
	
	
	//execute SIMPLE TA in round robin.
	
	
	
	//social: use degreeSet for sequential access and spos->getuserDegree(x) for random access
	//geographical: use getUserLocation and calculate dist for random access. getkNN for sequential access
	
	set<int> seen_users;
	set<int>::iterator ITseen;
	int round_robin = 0;
	res_point* newP = *itGEO;
	
	while(heap_size != k && newP != NULL && itSOC!=soc_set.end()){
		bool flag = false;
		double local_geo_score = 0, local_soc_score = 0, local_tfidf = 0;
		int user_id;
	
		if(round_robin%3 == 0){
			//Geo sorted access	
			user_id = newP->id;
			local_geo_score = 1 - ((double)newP->dist/(double)_radius);
			// cout <<"geo sort access - newP id: "<<newP->id<<" dist:  "<<newP->dist<<" | "<<newP->dist*EARTH_CIRCUMFERENCE/360 <<" | score = "<<local_geo_score<<endl;
			
			++itGEO;
			newP = *itGEO;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
			
				//Social RA
				auto it_soc_map = soc_map.find(user_id);
				if(it_soc_map != soc_map.end())
					local_soc_score = (double)((it_soc_map->second)->size())/(double)max_social;
				else
					local_soc_score = 0;
				
				//Keyword RA
				unordered_map<int,double>::iterator it = key_map.find(user_id);
				if(it != key_map.end()){
					local_tfidf = (*it).second;
				}
				else{
					local_tfidf = 0;
				}
			}
			
			
			
			last_geo_score = local_geo_score;
		}
		else if(round_robin%3 == 1){	
		
			//Social sorted access
			my_pair user = *itSOC;
			local_soc_score = (double)user.getScore()/(double)max_social;
			user_id = user.getId();
			++itSOC;
			
			ITseen = seen_users.find(user_id);
			if(ITseen == seen_users.end()){
				flag=true;
				seen_users.insert(user_id);
				
				//Geo RA
				res_point* localP = res_map[user_id];
				// cout <<"newPdist = "<<localP->dist<<" | "<<localP->dist*EARTH_CIRCUMFERENCE/360<<endl;
				local_geo_score = 1 - ((double)localP->dist/(double)_radius);
				
				//Keyword RA
				unordered_map<int,double>::iterator it = key_map.find(user_id);
				if(it != key_map.end()){
					local_tfidf = (*it).second;
				}
				else{
					local_tfidf = 0;
				}
			}
			
			last_soc_score = local_soc_score;
		}
		else if(round_robin%3 == 2){	
			while(true){
				if(itKEY != key_set->end()){
					//Keyword sorted access
					my_pair user = *itKEY;
					user_id = user.getId();
					local_tfidf = user.getScore();
					++itKEY;
					
					ITseen = seen_users.find(user_id);
					if(ITseen == seen_users.end()){
						
						//Geo RA
						auto it_res_map = res_map.find(user_id);
						if(it_res_map != res_map.end()){
							res_point* localP = res_map[user_id];
							local_geo_score = 1 - ((double)localP->dist/(double)_radius);
						}
						else{
							break;
						}
						
						//Social RA
						auto it_soc_map = soc_map.find(user_id);
						if(it_soc_map != soc_map.end())
							local_soc_score =(double)(it_soc_map->second)->size()/(double)max_social;
						else
							local_soc_score = 0;
							
							
						flag=true;
						seen_users.insert(user_id);
							
					}
				
					last_key_score = local_tfidf;
					break;
				}
				else{
					//SORTED ACCESS INTO GEO
					user_id = newP->id;
					local_geo_score = 1 - ((double)newP->dist/(double)_radius);
					++itGEO;
					newP = *itGEO;
					
					ITseen = seen_users.find(user_id);
					if(ITseen == seen_users.end()){
						flag=true;
						seen_users.insert(user_id);
					
						//Social RA
						auto it_soc_map = soc_map.find(user_id);
						if(it_soc_map != soc_map.end())
							local_soc_score = (double)(it_soc_map->second)->size()/(double)max_social;
						else
							local_soc_score = 0;		
					}
					
					local_tfidf = 0;
					last_key_score = 0;
					break;
				}
			}
		}
		
		if(flag==true)
			topk.insert(my_pair( user_id , local_soc_score + local_geo_score + local_tfidf));
		
		if(round_robin%3 == 2){
			// update threshold value
			threshold_value =  last_soc_score  + last_geo_score  + last_key_score  ;
			
			my_pair top1 =  *topk.begin();
			double score = top1.getScore();
			if(top1.getScore() > threshold_value){
				int id = top1.getId();
				//cout<<"adding to heap : user_id = "<<id<<" score = "<<score<<" threshold = "<<threshold_value<<endl;
				topk.erase(top1);
				Group* topG = new Group(soc_map[id]);
				topG->score = score;
				heap->push(topG);
				++heap_size;
			}
		}
		
		// cout<<"RR: "<<round_robin<<" user_id = "<<user_id<<" geo_score = "<<local_geo_score<<" social_score = "<<local_soc_score<<" tfidf = "<<local_tfidf<<" | threshold = "<<threshold_value<<endl;
	
		++round_robin;
	}
	// cout<<"RR end = "<<round_robin<<endl;

	return heap;

}