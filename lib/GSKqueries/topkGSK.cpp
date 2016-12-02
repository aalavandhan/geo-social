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

//gets underlying container of a priority queue
template <class T, class S, class C>
    S& Container(priority_queue<T, S, C>& q) {
        struct HackedQueue : private priority_queue<T, S, C> {
            static S& Container(priority_queue<T, S, C>& q) {
                return q.*&HackedQueue::c;
            }
        };
    return HackedQueue::Container(q);
}


template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

//hashes a pair into an unordered set
struct PairHasher
{
    size_t operator()(const pair<int, int> & v) const
    {
      size_t seed = 0;
      ::hash_combine(seed, v.first);
      ::hash_combine(seed, v.second);
      return seed;
    }
};


int topkGSK::getNumOfUsersChecked(){
	return usersChecked;
}

double topkGSK::print_time(struct timeval &start, struct timeval &end){
    double usec;
    usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    return usec;
}


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::NPRU(double x, double y,  unsigned int k, double w, vector<string> *terms){

	bool isDebug = false;
	// bool isDebug = true;

	priority_queue <QuadTreeNode*, vector<QuadTreeNode*>, QuadTreeNode::descending_score >* heap = new priority_queue <QuadTreeNode*, vector<QuadTreeNode*>, QuadTreeNode::descending_score >();

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* topk = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	usersChecked =0;
	threshold =0;

	std::priority_queue<double, std::vector<double>, std::greater<double> > my_min_heap;
	double best_score = -1;
	unsigned int size_of_min_heap = 0;

	vector <QuadTreeNode*>* rootNode = gpos->getQuadTree();
	// cout<<"Root level contains: "<<rootNode->size()<<" quad tree nodes."<<endl;
	//inverted index

    double query_length = 0;
	map<string, double> w_idf_map;
	// calculate query_length_squared;
	for(auto itt = terms->begin(); itt!=terms->end();++itt){
		int idf = gpos->getIDF(*itt);
		double idf_w = log10(DATASET_SIZE/(double)(idf));
		w_idf_map.insert((make_pair(*itt,idf_w)));
		query_length += idf_w*idf_w;
		// cout<<" idf: "<<idf<<endl;
		// cout<<"idfweight for term: "<<*itt<<" is "<<idf_w<<endl;
	}
	query_length = sqrt(query_length);

	for(auto it = w_idf_map.begin(); it!=w_idf_map.end();++it){
		it->second = it->second/query_length;
	}
	// cout<<"query length sqrt: "<<query_length<<endl;
	//load all top level nodes into the heap
	for(auto it = rootNode->begin(); it!=rootNode->end(); ++it){
		QuadTreeNode* qtn = *it;
		qtn->computeMinDist(x,y);

		double local_tf_idf = 0;
		for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
			auto idx_iter = qtn->idxImpact->find(*itt);			//get max_impact for term
			if(idx_iter != qtn->idxImpact->end()){
				double local_impact = idx_iter->second ;
				//max_impact*query_impact
				local_tf_idf += local_impact  * w_idf_map[*itt];
			}
		}
		qtn->setTFIDF(local_tf_idf);
		qtn->setType(QTN);
		qtn->computeScore();
		heap->push(qtn);
		// cout<<"\t\t\t QTN node_type: "<<qtn->getType() <<" level "<<qtn->level<<" max_degree "<<qtn->max_degree<<" minDist "<< qtn->c_minDist<<" max_tfidf "<<qtn->max_tfidf<<" bounding_score "<<qtn->bounding_score<<endl;
	}


    QuadTreeNode* tmp = heap->top();
    unsigned int count = 0;

    while ( !heap->empty() && count < k) {
        heap->pop();

        if (tmp->getType() == CELL){                               //it is a cell
			if(isDebug){
				cout<<"FOUND CELL"<<endl;
				cout<<"\t Node at level: "<<tmp->level<<" length: "<<tmp->length << "|  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "]" <<" bounding_score = "<<tmp->bounding_score<< endl;
			}
			Cell* c = gpos->getCell(tmp->getX1()+DELTA_X/2.0,tmp->getY1()+DELTA_Y/2.0);
			// cout<<"\t\tObtained cell: <"<<c->getIndexI()<<","<<c->getIndexJ()<<">"<<endl;

			unordered_map < int, double > tfidf_score_map;
			for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
				auto pl_impacts = c->lookupWordInImpacts(*it);
				if(pl_impacts == NULL)
					continue;

				for(auto it_pl = pl_impacts->begin(); it_pl!=pl_impacts->end();++it_pl){
					my_pair user = *it_pl;
					int user_id = user.getId();
					double user_impact = user.getScore();
					double partial_tfidf = user_impact * w_idf_map[*it];

					auto it_id = tfidf_score_map.find(user_id);
					if(it_id != tfidf_score_map.end()){
						it_id->second += partial_tfidf;
					}
					else{
						tfidf_score_map.insert(make_pair(user_id, partial_tfidf));
					}
				}
			}

			list<Point*>* L = c->getCheckIns();
            list<Point*>::iterator it;
            Point* p = NULL;
			// cout<<"\t\t\t points in cell : "<< L->size()<<endl;
            for(it=L->begin(); it != L->end(); ++it){
                p = *it;
				int u_id = p->getID();
                QuadTreeNode * qtn = new QuadTreeNode(p->getX(), p->getY(), p->getX(), p->getY(), 999, 999, NULL);	//length = 0 ; level = ??? ; parent node NULL
                qtn->setType(USER);
                qtn->setID(u_id);
                qtn->computeMinDist(x, y);
				qtn->max_degree = spos->getUserDegree(u_id);

				auto it_tfidf = tfidf_score_map.find(u_id);
				if(it_tfidf!=tfidf_score_map.end()){
					qtn->max_tfidf = it_tfidf->second;
				}
				else{
					qtn->max_tfidf =0;
				}

				double user_score = qtn->computeScore();
				if(user_score > best_score){
					if(size_of_min_heap == k){
						my_min_heap.pop();
						my_min_heap.push(user_score);
						best_score = my_min_heap.top();
						heap->push(qtn);
					}
					else{
						size_of_min_heap++;
						my_min_heap.push(user_score);
						heap->push(qtn);
					}
				}
				else{
					delete qtn;
				}
				++usersChecked;
				if(isDebug){
					cout <<"\t Inserting into heap user with id = "<<p->getID()<<" max_degree: "<< qtn->max_degree <<" Social Score: "<<qtn->max_degree/(double)MAXSC <<" TFIDF: "<< qtn->max_tfidf<<" DistScore: "<<(1 - (qtn->c_minDist/(double)MAXDIST))<<" Dist: "<< qtn->c_minDist*(EARTH_CIRCUMFERENCE/360)<< endl;
				}
            }
			delete tmp; //delete the reference to the placeholder cell
        }
        else if(tmp->getType() == QTN){                      //it is a quadtreenode: if level!=1 get children else get cell members
			if(isDebug){
				cout<<"FOUND QTN"<<endl;
				cout<<"\t Node at level: "<<tmp->level<<" length: "<<tmp->length << "|  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "]" <<" bounding_score = "<<tmp->bounding_score<<endl;
			}
			if(tmp->level != 1){
				// cout<<"QTN has level "<< tmp->level <<endl;
				for(int q = 0; q < HG_FANOUT; q++){
					QuadTreeNode* qtn = tmp->children->at(q);
					qtn->setType(QTN);
					qtn->computeMinDist(x, y);

					//given an intermediate node MBR_i, let tf_w_min represent the minimum squared summation of weighted terms of all users located within the bounds of MBR_i
					//have to guarantee that score of an MBR is greater than the score of any of its child/user
					//to achieve this bound we need to assume that the document length of a node is tf_w_min

					double local_tf_idf = 0;
					for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
						auto idx_iter = qtn->idxImpact->find(*itt);			//get max_impact for term
						if(idx_iter != qtn->idxImpact->end()){
							double local_impact = idx_iter->second ;
							//max_impact*query_impact
							local_tf_idf += local_impact  * w_idf_map[*itt];
						}
					}
					qtn->setTFIDF(local_tf_idf);
					if(qtn->computeScore() > best_score)
						heap->push(qtn);
					if(isDebug){
						cout<<"\t\t\t en-queuing child QTN: at level "<<qtn->level<<" max_degree: "<< qtn->max_degree <<" Social Score: "<<qtn->max_degree/(double)MAXSC <<" TFIDF: "<< qtn->max_tfidf<<" DistScore: "<<(1 - (qtn->c_minDist/(double)MAXDIST))<<" Dist: "<< qtn->c_minDist*(EARTH_CIRCUMFERENCE/360)<< "|  x1 "<<qtn->getX1()<<" y1 "<<qtn->getY1()<<" x2 "<<qtn->getX2()<<" y2 "<<qtn->getY2()<<" range covers is: ["<< ((qtn->getX1() - MIN_X)/DELTA_X)<< ", " << ((qtn->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((qtn->getX2() - MIN_X)/DELTA_X) << ", " << ((qtn->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
					}
				}
			}
			else{
				for(int q = 0; q < HG_FANOUT; q++){
					Cell* c = tmp->cellMembers->at(q);

					if(c->getCheckIns()->size() == 0)
						continue;

					QuadTreeNode* qtn = new QuadTreeNode(c->getX1(),c->getY1(),c->getX2(),c->getY2(), 0 , 1, NULL); // do not give reference to parent since this node will get deleted at query termination
					qtn->setType(CELL);

					//compute geo score
					qtn->computeMinDist(x, y);
					qtn->max_degree = c->max_degree;

					//we are takign out a qtn at level 1 and inserting the 4 cells in its lower level

					//just calculate max possible tfidf of the cell and insert this qtn object
					double local_tf_idf = 0;
					for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
						auto idx_iter =c->idxImpact->find(*itt);			//get max_impact for term
						if(idx_iter != c->idxImpact->end()){
							double local_impact = idx_iter->second ;
							//max_impact*query_impact
							local_tf_idf += local_impact  * w_idf_map[*itt];
						}
					}
					qtn->setTFIDF(local_tf_idf);
					if(qtn->computeScore() > best_score)
						heap->push(qtn);
					else
						delete qtn;
					if(isDebug){
						cout<<"\t\t\t en-queuing child Cell: <"<<c->getIndexI()<<","<<c->getIndexJ()<<" max_degree: "<< qtn->max_degree <<" Social Score: "<<qtn->max_degree/(double)MAXSC <<" TFIDF: "<< qtn->max_tfidf<<" DistScore: "<<(1 - (qtn->c_minDist/(double)MAXDIST))<<" Dist: "<< qtn->c_minDist*(EARTH_CIRCUMFERENCE/360)<< "|  x1 "<<qtn->getX1()<<" y1 "<<qtn->getY1()<<" x2 "<<qtn->getX2()<<" y2 "<<qtn->getY2()<<" range covers is: ["<< ((qtn->getX1() - MIN_X)/DELTA_X)<< ", " << ((qtn->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((qtn->getX2() - MIN_X)/DELTA_X) << ", " << ((qtn->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
					}
				}
			}
			//do not delete tmp since it is a part of the quad tree
        }
        else{                                                                 // it is a user
            //v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            //v.setBestDist(best_dist);
			// cout<<"FOUND USER"<<endl;
            res_point* rp = new res_point();
            rp->id = tmp->getID();
            rp->x = tmp->getX1();
            rp->y = tmp->getY1();
            rp->dist = tmp->getMinDist();

			// if(isDebug){
				// cout << "\t\t RESULT User ID: " << rp->id <<" max_degree: "<< tmp->max_degree <<" Social Score: "<<tmp->max_degree/(double)MAXSC <<" TFIDF: "<< tmp->max_tfidf<<" DistScore: "<<(1 - (rp->dist/(double)MAXDIST))<<" Dist: "<< rp->dist*(EARTH_CIRCUMFERENCE/360)<<" MAXDIST: "<<MAXDIST*(EARTH_CIRCUMFERENCE/360)<<" km"<< endl;
        cout << rp->id <<"," << rp->dist*(EARTH_CIRCUMFERENCE/360) << "," << rp->x << "," << rp->y << "," << MAXDIST*(EARTH_CIRCUMFERENCE/360) << "," << MAXDIST*(EARTH_CIRCUMFERENCE/360) << "," << tmp->max_degree/(double)MAXSC << "," << tmp->max_degree;
			// }

			Group* tempG = (Group *)malloc(sizeof(Group));
			Group* newG = new(tempG) Group(rp);
			newG->score = tmp->bounding_score;
            topk->push(newG);
            count++;
            delete tmp;		//delete placeholder
        }
        tmp = heap->top();
    }

    // delete the rest of the heap
    while(!heap->empty()) {
        QuadTreeNode* qtn = heap->top();
        if(qtn->getType() != QTN)
            delete qtn;
        heap->pop();
    }
    delete heap;

	cout<<"Users Explored by HA: "<<usersChecked<<endl;
	return topk;

}

priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::NSTP_VIS(double x, double y,  unsigned int k, double w, int USER_, vector<string> *terms){

	priority_queue <QuadTreeNode*, vector<QuadTreeNode*>, QuadTreeNode::descending_score >* heap = new priority_queue <QuadTreeNode*, vector<QuadTreeNode*>, QuadTreeNode::descending_score >();

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* topk = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	usersChecked =0;
	threshold =0;

	// struct timeval startInHeap,endInHeap, startOutHeap, endOutHeap;
	// double sumInHeap = 0, sumOutHeap = 0;
	// int numElementsPopped=0,numElementsInserted=0;

	std::priority_queue<double, std::vector<double>, std::greater<double> > my_min_heap;
	double best_score = -1;
	unsigned int size_of_min_heap = 0;

	int* friends;
	unsigned int friendsSize;
	spos->getFriends(USER_, friends, friendsSize);
	// vector<char*> FR;
	// cout<<"Friends of input user: "<<endl;
	for(unsigned int i = 0; i < friendsSize; i++){
		// ostringstream ss;
		// ss<<friends[i];
		// string f_id_s = ss.str();
		// FR.push_back(strdup(f_id_s.c_str()));
		// cout<<friends[i]<<" ";
	}
	cout<<"\n";
	// cout<<endl;

	// for each user in friends check if he belongs in the other set
	// cout<<"User: "<<USER_<<" has friends "<<friendsSize<<endl;
	// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
		// char * fid = *fit;
		// cout<<fid<<" ";
	// }

	vector <QuadTreeNode*>* rootNode = gpos->getQuadTree();
	// cout<<"Root level contains: "<<rootNode->size()<<" quad tree nodes."<<endl;
	//inverted index

    double query_length = 0;
	map<string, double> w_idf_map;
	// calculate query_length_squared;
	for(auto itt = terms->begin(); itt!=terms->end();++itt){
		int idf = gpos->getP_IDF(*itt);
		double idf_w = log10(POI_SIZE/(double)(idf));
		w_idf_map.insert((make_pair(*itt,idf_w)));
		query_length += idf_w*idf_w;
		// cout<<" idf: "<<idf<<endl;
		// cout<<"idfweight for term: "<<*itt<<" is "<<idf_w<<endl;
	}
	query_length = sqrt(query_length);

	for(auto it = w_idf_map.begin(); it!=w_idf_map.end();++it){
		it->second = it->second/query_length;
	}
	// cout<<"query length sqrt: "<<query_length<<endl;
	//load all top level nodes into the heap
	for(auto it = rootNode->begin(); it!=rootNode->end(); ++it){
		QuadTreeNode* qtn = *it;
		qtn->computeMinDist(x,y);

		double local_tf_idf = 0;
		for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
			auto idx_iter = qtn->P_idxImpact->find(*itt);			//get max_impact for term
			if(idx_iter != qtn->P_idxImpact->end()){
				double local_impact = idx_iter->second ;
				//max_impact*query_impact
				// cout<<"impact of term: "<<*itt<<" is "<< local_impact<<endl;
				local_tf_idf += local_impact  * w_idf_map[*itt];
			}
		}
		// cout<<"Total tfidf: "<<local_tf_idf<<endl;

		// Calculate vnp from true bloom filter
		int vnp = 0;
		if(qtn->filter!=NULL){
			for(unsigned int i = 0; i < friendsSize; i++){
				int f_id = friends[i];
				// check if friend is in bloom filter of qtn
				if(qtn->filter->contains(f_id)){
					++vnp;
				}
			}
		}
		// cout<<"Root VNP: "<<vnp<<endl;
		//FROM SCALING BLOOM FILTER
		// int vnp = 0;
		// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
			// scaling_bloom_check(qtn->bloom, *fit, strlen(*fit));
			// ++vnp;
		// }

		qtn->setBloomVNP((double)vnp/(double)friendsSize);
		qtn->setTFIDF(local_tf_idf);
		qtn->setType(QTN);
		qtn->computePOIScore();
		heap->push(qtn);
		// cout<<"\t\t\t QTN node_type: "<<qtn->getType() <<" level "<<qtn->level<<" max_degree "<<qtn->max_degree<<" minDist "<< qtn->c_minDist<<" max_tfidf "<<qtn->max_tfidf<<" VNP "<<vnp<<" bounding_score "<<qtn->bounding_score<<endl;
	}


    QuadTreeNode* tmp = heap->top();
    unsigned int count = 0;

    while ( !heap->empty() && count < k) {
		// ++numElementsPopped;
		// gettimeofday(&startOutHeap, NULL);
		heap->pop();
		// gettimeofday(&endOutHeap, NULL);
		// sumOutHeap+=print_time(startOutHeap, endOutHeap);


        if (tmp->getType() == CELL){                               //it is a cell
			// cout<<"FOUND CELL"<<endl;
			// cout<<"\t Node at level: "<<tmp->level<<" length: "<<tmp->length << "|  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "]" <<" bounding_score = "<<tmp->bounding_score<< endl;
			Cell* c = gpos->getCell(tmp->getX1()+DELTA_X/2.0,tmp->getY1()+DELTA_Y/2.0);
			// cout<<"\t\tObtained cell: <"<<c->getIndexI()<<","<<c->getIndexJ()<<">"<<endl;

			unordered_map < int, double > tfidf_score_map;
			for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
				auto pl_impacts = c->lookupWordIn_P_Impacts(*it);
				if(pl_impacts == NULL)
					continue;

				for(auto it_pl = pl_impacts->begin(); it_pl!=pl_impacts->end();++it_pl){
					my_pair user = *it_pl;
					int user_id = user.getId();
					double user_impact = user.getScore();
					double partial_tfidf = user_impact * w_idf_map[*it];

					auto it_id = tfidf_score_map.find(user_id);
					if(it_id != tfidf_score_map.end()){
						it_id->second += partial_tfidf;
					}
					else{
						tfidf_score_map.insert(make_pair(user_id, partial_tfidf));
					}

				}
			}


            list<Point*>* points_of_interest = c->getPointsOfInterest();
            list<Point*>::iterator it;
            Point* p = NULL;
			// cout<<"\t\t\t points_of_interest in cell : "<< points_of_interest->size()<<endl;
            for(it=points_of_interest->begin(); it != points_of_interest->end(); ++it){
                p = *it;
				int poi_id = p->getID();
                QuadTreeNode * qtn = new QuadTreeNode(p->getX(), p->getY(), p->getX(), p->getY(), 999, 999, NULL);	//length = 0 ; level = -1 ; parent node NULL
                qtn->setType(USER);
                qtn->setID(poi_id);
                qtn->computeMinDist(x, y);

				//TODO : calculate vnp from actual lists
				int vnp = 0;


				auto ut = (c->P_historyMap).find(poi_id);
				if(ut != c->P_historyMap.end()){
					unordered_set<int>* history_set = ut->second;
					for(unsigned int i = 0; i < friendsSize; i++){
						int f_id = friends[i];
						//check if friend is in bloom filter of qtn
						if(history_set->find(f_id)!=history_set->end()){
							++vnp;
						}
					}
				}

				// cout<<" POI : "<<poi_id<<" has VNP: "<<vnp<<endl;

				qtn->setBloomVNP((double)vnp/(double)friendsSize);

				auto it_tfidf = tfidf_score_map.find(poi_id);
				if(it_tfidf!=tfidf_score_map.end()){
					qtn->max_tfidf = it_tfidf->second;
				}
				else{
					qtn->max_tfidf =0;
				}

				// cout<<"\t POI id: "<< p->getID() <<" VNP: "<<vnp<<" TFIDF: "<<qtn->max_tfidf<<" dist Score: "<<(1 - qtn->c_minDist/(double)MAXDIST)<<" \n";
				double poi_score = qtn->computePOIScore();
				if(poi_score > best_score){
					if(size_of_min_heap == k){
						my_min_heap.pop();
						my_min_heap.push(poi_score);
						best_score = my_min_heap.top();
						heap->push(qtn);
					}
					else{
						size_of_min_heap++;
						my_min_heap.push(poi_score);
						heap->push(qtn);
					}
				}
				else{
					delete qtn;
				}
				++usersChecked;
				// cout <<"\t Inserting into heap POI with id = "<<p->getID()<<" VNP: "<<vnp<<" with degree "<< qtn->max_degree <<" minDist "<< qtn->c_minDist<<" max_tfidf "<<qtn->max_tfidf<<" bounding_score "<<qtn->bounding_score<<endl;
            }

			//delete the reference to the placeholder cell
			delete tmp;
        }
        else if(tmp->getType() == QTN){                      //it is a quadtreenode: if level!=1 get children else get cell members
			// cout<<"FOUND QTN"<<endl;
			// cout<<"\t Node at level: "<<tmp->level<<" length: "<<tmp->length << "|  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "]" <<" bounding_score = "<<tmp->bounding_score<<endl;
			// numElementsInserted+=HG_FANOUT;
			if(tmp->level != 1){
				// cout<<"QTN has level "<< tmp->level <<endl;
				for(int q = 0; q < HG_FANOUT; q++){
					QuadTreeNode* qtn = tmp->children->at(q);
					qtn->setType(QTN);
					qtn->computeMinDist(x, y);

					//from true bloom filter
					int vnp = 0;
					if(qtn->filter!=NULL){
						for(unsigned int i = 0; i < friendsSize; i++){
							int f_id = friends[i];
							//check if friend is in bloom filter of qtn
							if(qtn->filter->contains(f_id)){
								++vnp;
							}
						}
					}

					//from scaling bloom filters
					// int vnp = 0;
					// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
						// scaling_bloom_check(qtn->bloom, *fit, strlen(*fit));
						// ++vnp;
					// }

					qtn->setBloomVNP((double)vnp/(double)friendsSize);

					double local_tf_idf = 0;
					for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
						auto idx_iter = qtn->P_idxImpact->find(*itt);			//get max_impact for term
						if(idx_iter != qtn->P_idxImpact->end()){
							double local_impact = idx_iter->second ;
							//max_impact*query_impact
							local_tf_idf += local_impact  * w_idf_map[*itt];
						}
					}
					qtn->setTFIDF(local_tf_idf);

					if(qtn->computePOIScore() > best_score){
						// gettimeofday(&startInHeap, NULL);
						heap->push(qtn);
						// gettimeofday(&endInHeap, NULL);
						// sumInHeap+=print_time(startInHeap, endInHeap);
					}
					// cout<<"\t\t\t en-queuing child QTN: at level "<<qtn->level<<" minDist = "<<qtn->c_minDist<<" VNP: "<<vnp<<" max_degree = "<<qtn->max_degree<<" max_tfidf = "<<qtn->max_tfidf<<" bounding_score = "<<qtn->bounding_score<<endl;
				}
			}
			else{
				for(int q = 0; q < HG_FANOUT; q++){
					Cell* c = tmp->cellMembers->at(q);

					if(c->getPointsOfInterest()->size() == 0)
						continue;

					QuadTreeNode* qtn = new QuadTreeNode(c->getX1(),c->getY1(),c->getX2(),c->getY2(), 0 , 1, NULL); // do not give reference to parent since this node will get deleted at query termination
					qtn->setType(CELL);

					//compute geo score
					qtn->computeMinDist(x, y);
					qtn->max_degree = c->max_degree;

					//from true bloom filter
					int vnp = 0;
					if(c->filter!=NULL){
						for(unsigned int i = 0; i < friendsSize; i++){
							int f_id = friends[i];
							// check if friend is in bloom filter of c
							if(c->filter->contains(f_id)){
								++vnp;
							}
						}
					}

					//from scaling bloom filters
					// int vnp = 0;
					// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
						// scaling_bloom_check(c->bloom, *fit, strlen(*fit));
						// ++vnp;
					// }
					qtn->setBloomVNP((double)vnp/(double)friendsSize);
					//we are takign out a qtn at level 1 and inserting the 4 leaf cells in its lower level

					//just calculate max possible tfidf of the cell and insert this qtn object
					double local_tf_idf = 0;
					for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
						auto idx_iter =c->P_idxImpact->find(*itt);			//get max_impact for term
						if(idx_iter != c->P_idxImpact->end()){
							double local_impact = idx_iter->second ;
							//max_impact*query_impact
							local_tf_idf += local_impact  * w_idf_map[*itt];
						}
					}
					qtn->setTFIDF(local_tf_idf);

					if(qtn->computePOIScore() > best_score){
						// gettimeofday(&startInHeap, NULL);
						heap->push(qtn);
						// gettimeofday(&endInHeap, NULL);
						// sumInHeap+=print_time(startInHeap, endInHeap);
					}else
						delete qtn;
					// cout<<"\t\t inserting into heap cell: <"<<c->getIndexI()<<","<<c->getIndexJ()<<" VNP: "<<vnp<<" max_degree "<< qtn->max_degree <<" max_tfidf = "<<qtn->max_tfidf<<" bounding_score = "<<qtn->bounding_score<<endl;
				}
			}
			//do not delete tmp since it is a part of the quad tree
        }
        else{                                                                 // it is a user
            //v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            //v.setBestDist(best_dist);
			// cout<<"FOUND USER"<<endl;
            res_point* rp = new res_point();
            rp->id = tmp->getID();
            rp->x = tmp->getX1();
            rp->y = tmp->getY1();
            rp->dist = tmp->getMinDist();

			// cout<<"\n";
            cout << rp->id <<","<< tmp->vnp_score <<","<< tmp->max_tfidf <<"," << (1 - rp->dist/(double)MAXDIST)<<","<< rp->dist*(EARTH_CIRCUMFERENCE/360) <<"," << rp->x << "," << rp->y << endl;
			// cout<<"\n";
			Group* tempG = (Group *)malloc(sizeof(Group));
			Group* newG = new(tempG) Group(rp);
			newG->score = tmp->bounding_score;
            topk->push(newG);
            count++;
            delete tmp;		//delete placeholder
        }
        tmp = heap->top();
    }

    // delete the rest of the heap
    while(!heap->empty()) {
        QuadTreeNode* qtn = heap->top();
        if(qtn->getType() != QTN)
            delete qtn;
        heap->pop();
    }
    delete heap;
	// cout<<"Time taken to insert qtn's into heap: "<<sumInHeap/1000<<" ms\n";
	// cout<<"Number of elements popped: "<<numElementsPopped<<"\n";
	// cout<<"Number of elements inserted: "<<numElementsInserted<<"\n";
	// cout<<"Number of users Explored by HA: "<<usersChecked<<"\n";
	return topk;

}


priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::NSTP(double x, double y,  unsigned int k, double w, int USER_, vector<string> *terms){

	priority_queue <QuadTreeNode*, vector<QuadTreeNode*>, QuadTreeNode::descending_score >* heap = new priority_queue <QuadTreeNode*, vector<QuadTreeNode*>, QuadTreeNode::descending_score >();

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* topk = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	usersChecked =0;
	threshold =0;

	// struct timeval startInHeap,endInHeap, startOutHeap, endOutHeap;
	// double sumInHeap = 0, sumOutHeap = 0;
	// int numElementsPopped=0,numElementsInserted=0;

	std::priority_queue<double, std::vector<double>, std::greater<double> > my_min_heap;
	double best_score = -1;
	unsigned int size_of_min_heap = 0;

	int* friends;
	unsigned int friendsSize;
	spos->getFriends(USER_, friends, friendsSize);
	// vector<char*> FR;
	// cout<<"Friends: "<<endl;
	// for(unsigned int i = 0; i < friendsSize; i++){
		// ostringstream ss;
		// ss<<friends[i];
		// string f_id_s = ss.str();
		// FR.push_back(strdup(f_id_s.c_str()));
		// cout<<friends[i]<<" ";
	// }
	// cout<<endl;

	// for each user in friends check if he belongs in the other set
	// cout<<"User: "<<USER_<<" has friends "<<friendsSize<<endl;
	// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
		// char * fid = *fit;
		// cout<<fid<<" ";
	// }

	vector <QuadTreeNode*>* rootNode = gpos->getQuadTree();
	// cout<<"Root level contains: "<<rootNode->size()<<" quad tree nodes."<<endl;
	//inverted index

    double query_length = 0;
	map<string, double> w_idf_map;
	// calculate query_length_squared;
	for(auto itt = terms->begin(); itt!=terms->end();++itt){
		int idf = gpos->getP_IDF(*itt);
		double idf_w = log10(POI_SIZE/(double)(idf));
		w_idf_map.insert((make_pair(*itt,idf_w)));
		query_length += idf_w*idf_w;
		// cout<<" idf: "<<idf<<endl;
		// cout<<"idfweight for term: "<<*itt<<" is "<<idf_w<<endl;
	}
	query_length = sqrt(query_length);

	for(auto it = w_idf_map.begin(); it!=w_idf_map.end();++it){
		it->second = it->second/query_length;
	}
	// cout<<"query length sqrt: "<<query_length<<endl;
	//load all top level nodes into the heap
	for(auto it = rootNode->begin(); it!=rootNode->end(); ++it){
		QuadTreeNode* qtn = *it;
		qtn->computeMinDist(x,y);

		double local_tf_idf = 0;
		for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
			auto idx_iter = qtn->P_idxImpact->find(*itt);			//get max_impact for term
			if(idx_iter != qtn->P_idxImpact->end()){
				double local_impact = idx_iter->second ;
				//max_impact*query_impact
				// cout<<"impact of term: "<<*itt<<" is "<< local_impact<<endl;
				local_tf_idf += local_impact  * w_idf_map[*itt];
			}
		}
		// cout<<"Total tfidf: "<<local_tf_idf<<endl;

		// Calculate vnp from true bloom filter
		int vnp = 0;
		if(qtn->filter!=NULL){
			for(unsigned int i = 0; i < friendsSize; i++){
				int f_id = friends[i];
				// check if friend is in bloom filter of qtn
				if(qtn->filter->contains(f_id)){
					++vnp;
				}
			}
		}
		// cout<<"Root VNP: "<<vnp<<endl;
		//FROM SCALING BLOOM FILTER
		// int vnp = 0;
		// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
			// scaling_bloom_check(qtn->bloom, *fit, strlen(*fit));
			// ++vnp;
		// }

		qtn->setBloomVNP((double)vnp/(double)friendsSize);
		qtn->setTFIDF(local_tf_idf);
		qtn->setType(QTN);
		qtn->computePOIScore();
		heap->push(qtn);
		// cout<<"\t\t\t QTN node_type: "<<qtn->getType() <<" level "<<qtn->level<<" max_degree "<<qtn->max_degree<<" minDist "<< qtn->c_minDist<<" max_tfidf "<<qtn->max_tfidf<<" VNP "<<vnp<<" bounding_score "<<qtn->bounding_score<<endl;
	}


    QuadTreeNode* tmp = heap->top();
    unsigned int count = 0;

    while ( !heap->empty() && count < k) {
		// ++numElementsPopped;
		// gettimeofday(&startOutHeap, NULL);
		heap->pop();
		// gettimeofday(&endOutHeap, NULL);
		// sumOutHeap+=print_time(startOutHeap, endOutHeap);


        if (tmp->getType() == CELL){                               //it is a cell
			// cout<<"FOUND CELL"<<endl;
			// cout<<"\t Node at level: "<<tmp->level<<" length: "<<tmp->length << "|  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "]" <<" bounding_score = "<<tmp->bounding_score<< endl;
			Cell* c = gpos->getCell(tmp->getX1()+DELTA_X/2.0,tmp->getY1()+DELTA_Y/2.0);
			// cout<<"\t\tObtained cell: <"<<c->getIndexI()<<","<<c->getIndexJ()<<">"<<endl;

			unordered_map < int, double > tfidf_score_map;
			for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
				auto pl_impacts = c->lookupWordIn_P_Impacts(*it);
				if(pl_impacts == NULL)
					continue;

				for(auto it_pl = pl_impacts->begin(); it_pl!=pl_impacts->end();++it_pl){
					my_pair user = *it_pl;
					int user_id = user.getId();
					double user_impact = user.getScore();
					double partial_tfidf = user_impact * w_idf_map[*it];

					auto it_id = tfidf_score_map.find(user_id);
					if(it_id != tfidf_score_map.end()){
						it_id->second += partial_tfidf;
					}
					else{
						tfidf_score_map.insert(make_pair(user_id, partial_tfidf));
					}

				}
			}


            list<Point*>* points_of_interest = c->getPointsOfInterest();
            list<Point*>::iterator it;
            Point* p = NULL;
			// cout<<"\t\t\t points_of_interest in cell : "<< points_of_interest->size()<<endl;
            for(it=points_of_interest->begin(); it != points_of_interest->end(); ++it){
                p = *it;
				int poi_id = p->getID();
                QuadTreeNode * qtn = new QuadTreeNode(p->getX(), p->getY(), p->getX(), p->getY(), 999, 999, NULL);	//length = 0 ; level = -1 ; parent node NULL
                qtn->setType(USER);
                qtn->setID(poi_id);
                qtn->computeMinDist(x, y);

				//TODO : calculate vnp from actual lists
				int vnp = 0;
				auto ut = (c->P_historyMap).find(poi_id);
				if(ut != c->P_historyMap.end()){
					unordered_set<int>* history_set = ut->second;
					for(unsigned int i = 0; i < friendsSize; i++){
						int f_id = friends[i];
						//check if friend is in bloom filter of qtn
						if(history_set->find(f_id)!=history_set->end()){
							++vnp;
						}
					}
				}
				// cout<<" POI : "<<poi_id<<" has VNP: "<<vnp<<endl;

				qtn->setBloomVNP((double)vnp/(double)friendsSize);

				auto it_tfidf = tfidf_score_map.find(poi_id);
				if(it_tfidf!=tfidf_score_map.end()){
					qtn->max_tfidf = it_tfidf->second;
				}
				else{
					qtn->max_tfidf =0;
				}

				// cout<<"\t POI id: "<< p->getID() <<" VNP: "<<vnp<<" TFIDF: "<<qtn->max_tfidf<<" dist Score: "<<(1 - qtn->c_minDist/(double)MAXDIST)<<" \n";
				double poi_score = qtn->computePOIScore();
				if(poi_score > best_score){
					if(size_of_min_heap == k){
						my_min_heap.pop();
						my_min_heap.push(poi_score);
						best_score = my_min_heap.top();
						heap->push(qtn);
					}
					else{
						size_of_min_heap++;
						my_min_heap.push(poi_score);
						heap->push(qtn);
					}
				}
				else{
					delete qtn;
				}
				++usersChecked;
				// cout <<"\t Inserting into heap POI with id = "<<p->getID()<<" with degree "<< qtn->max_degree <<" minDist "<< qtn->c_minDist<<" max_tfidf "<<qtn->max_tfidf<<" bounding_score "<<qtn->bounding_score<<endl;
            }

			//delete the reference to the placeholder cell
			delete tmp;
        }
        else if(tmp->getType() == QTN){                      //it is a quadtreenode: if level!=1 get children else get cell members
			// cout<<"FOUND QTN"<<endl;
			// cout<<"\t Node at level: "<<tmp->level<<" length: "<<tmp->length << "|  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "]" <<" bounding_score = "<<tmp->bounding_score<<endl;
			// numElementsInserted+=HG_FANOUT;
			if(tmp->level != 1){
				// cout<<"QTN has level "<< tmp->level <<endl;
				for(int q = 0; q < HG_FANOUT; q++){
					QuadTreeNode* qtn = tmp->children->at(q);
					qtn->setType(QTN);
					qtn->computeMinDist(x, y);

					//from true bloom filter
					int vnp = 0;
					if(qtn->filter!=NULL){
						for(unsigned int i = 0; i < friendsSize; i++){
							int f_id = friends[i];
							//check if friend is in bloom filter of qtn
							if(qtn->filter->contains(f_id)){
								++vnp;
							}
						}
					}

					//from scaling bloom filters
					// int vnp = 0;
					// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
						// scaling_bloom_check(qtn->bloom, *fit, strlen(*fit));
						// ++vnp;
					// }

					qtn->setBloomVNP((double)vnp/(double)friendsSize);

					double local_tf_idf = 0;
					for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
						auto idx_iter = qtn->P_idxImpact->find(*itt);			//get max_impact for term
						if(idx_iter != qtn->P_idxImpact->end()){
							double local_impact = idx_iter->second ;
							//max_impact*query_impact
							local_tf_idf += local_impact  * w_idf_map[*itt];
						}
					}
					qtn->setTFIDF(local_tf_idf);

					if(qtn->computePOIScore() > best_score){
						// gettimeofday(&startInHeap, NULL);
						heap->push(qtn);
						// gettimeofday(&endInHeap, NULL);
						// sumInHeap+=print_time(startInHeap, endInHeap);
					}
					// cout<<"\t\t\t en-queuing child QTN: at level "<<qtn->level<<" minDist = "<<qtn->c_minDist<<" max_degree = "<<qtn->max_degree<<" max_tfidf = "<<qtn->max_tfidf<<" bounding_score = "<<qtn->bounding_score<<endl;
				}
			}
			else{
				for(int q = 0; q < HG_FANOUT; q++){
					Cell* c = tmp->cellMembers->at(q);

					if(c->getPointsOfInterest()->size() == 0)
						continue;

					QuadTreeNode* qtn = new QuadTreeNode(c->getX1(),c->getY1(),c->getX2(),c->getY2(), 0 , 1, NULL); // do not give reference to parent since this node will get deleted at query termination
					qtn->setType(CELL);

					//compute geo score
					qtn->computeMinDist(x, y);
					qtn->max_degree = c->max_degree;

					//from true bloom filter
					int vnp = 0;
					if(c->filter!=NULL){
						for(unsigned int i = 0; i < friendsSize; i++){
							int f_id = friends[i];
							// check if friend is in bloom filter of c
							if(c->filter->contains(f_id)){
								++vnp;
							}
						}
					}

					//from scaling bloom filters
					// int vnp = 0;
					// for(auto fit = FR.begin(); fit!=FR.end(); ++fit){
						// scaling_bloom_check(c->bloom, *fit, strlen(*fit));
						// ++vnp;
					// }
					qtn->setBloomVNP((double)vnp/(double)friendsSize);
					//we are takign out a qtn at level 1 and inserting the 4 leaf cells in its lower level

					//just calculate max possible tfidf of the cell and insert this qtn object
					double local_tf_idf = 0;
					for(auto itt = terms->begin(); itt!=terms->end();++itt){	//for each term calculate
						auto idx_iter =c->P_idxImpact->find(*itt);			//get max_impact for term
						if(idx_iter != c->P_idxImpact->end()){
							double local_impact = idx_iter->second ;
							//max_impact*query_impact
							local_tf_idf += local_impact  * w_idf_map[*itt];
						}
					}
					qtn->setTFIDF(local_tf_idf);

					if(qtn->computePOIScore() > best_score){
						// gettimeofday(&startInHeap, NULL);
						heap->push(qtn);
						// gettimeofday(&endInHeap, NULL);
						// sumInHeap+=print_time(startInHeap, endInHeap);
					}else
						delete qtn;
					// cout<<"\t\t inserting into heap cell: <"<<c->getIndexI()<<","<<c->getIndexJ()<<" max_degree "<< qtn->max_degree <<" max_tfidf = "<<qtn->max_tfidf<<" VNP: "<<vnp<<" bounding_score = "<<qtn->bounding_score<<endl;
				}
			}
			//do not delete tmp since it is a part of the quad tree
        }
        else{                                                                 // it is a user
            //v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            //v.setBestDist(best_dist);
			// cout<<"FOUND USER"<<endl;
            res_point* rp = new res_point();
            rp->id = tmp->getID();
            rp->x = tmp->getX1();
            rp->y = tmp->getY1();
            rp->dist = tmp->getMinDist();

			// cout<<"\n";
            // cout << "\t\t RESULT POI ID: " << rp->id <<" VNP: "<< tmp->vnp_score <<" TFIDF: "<< tmp->max_tfidf<<" DistScore: "<<(1 - rp->dist/(double)MAXDIST)<<" Dist: "<< rp->dist*(EARTH_CIRCUMFERENCE/360)<<" km"<< endl;
			// cout<<"\n";
			Group* tempG = (Group *)malloc(sizeof(Group));
			Group* newG = new(tempG) Group(rp);
			newG->score = tmp->bounding_score;
            topk->push(newG);
            count++;
            delete tmp;		//delete placeholder
        }
        tmp = heap->top();
    }

    // delete the rest of the heap
    while(!heap->empty()) {
        QuadTreeNode* qtn = heap->top();
        if(qtn->getType() != QTN)
            delete qtn;
        heap->pop();
    }
    delete heap;
	// cout<<"Time taken to insert qtn's into heap: "<<sumInHeap/1000<<" ms\n";
	// cout<<"Number of elements popped: "<<numElementsPopped<<"\n";
	// cout<<"Number of elements inserted: "<<numElementsInserted<<"\n";
	// cout<<"Number of users Explored by HA: "<<usersChecked<<"\n";
	return topk;

}




struct IntCmp {
	bool operator()(const std::pair<std::string, int> &lhs, const std::pair<std::string, int> &rhs) {
		return lhs.second < rhs.second;
	}
};

class tc_pair{

private:
    string word;
    int occur_t;

public:
    tc_pair(string _word, int _occur_t): word(_word), occur_t(_occur_t){}

    string getWD() const {return word;}
    int getOT() const {return occur_t;}
};


struct pair_descending_counter{
    bool operator()(const tc_pair  &_x, const tc_pair  &_y){
        return _y.getOT() < _x.getOT();
    }
};



priority_queue <Group*, vector<Group*>, Group::ascending_score >* topkGSK::FSKR(double x, double y, unsigned int k, double w, double _radius){

	priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap = new priority_queue <Group*, vector<Group*>, Group::ascending_score >();
	std::unordered_map<string,int> counters;

	std::vector<std::pair<std::string, int>> posting_list_size_sorted;
	vector<pair<int, int> > FrConFood;

	int best_score = -1;
	// struct timeval startGEO, endGEO, startALG, endALG;

	// gettimeofday(&startGEO, NULL);
	unordered_map<string, unordered_set<int>*>* posting_list_map = new unordered_map<string, unordered_set<int>*>();
	unordered_map<int, res_point*>* usersInValidRange = (unordered_map<int, res_point*>*) gpos->getIDXinRange(x, y, _radius, (void*)posting_list_map);
	usersChecked = usersInValidRange->size();
	// gettimeofday(&endGEO, NULL);
	// cout<<"Number of users in range: "<<usersInValidRange->size()<<endl;
	// cout<<"Total Time for getRangeWithIDX: "<<print_time(startGEO, endGEO)/1000<<" ms"<<endl;
	multiset<tc_pair, pair_descending_counter> topk;
	multiset<tc_pair, pair_descending_counter>::iterator topk_iter;
	// gettimeofday(&startALG, NULL);

	int* friends;
	unsigned int friendsSize;
	unordered_map<int, pair<int*,unsigned int> > soc_map;

	for(auto g_iter = posting_list_map->begin(); g_iter != posting_list_map->end(); ++g_iter){
		posting_list_size_sorted.push_back(make_pair(g_iter->first,(g_iter->second)->size()));
	}
	sort(posting_list_size_sorted.begin(), posting_list_size_sorted.end(), IntCmp());

	for(auto g_iter = posting_list_size_sorted.begin(); g_iter != posting_list_size_sorted.end(); ++g_iter){
		pair<string,int> t_pair = *g_iter;
		string word = t_pair.first;
		int size = t_pair.second;

		if((size*(size-1)) <= best_score){
			break;
		}

		auto shit = posting_list_map->find(word);
		unordered_set<int>* pl = shit->second;
		for (auto wut = pl->cbegin(); wut != pl->cend() /* not hoisted */; /* no increment */)
		{
			if(usersInValidRange->find(*wut)==usersInValidRange->end()){
				pl->erase(wut++);
		    }
		    else
		    {
				++wut;
		    }
		}
		int word_counter = 0;
		// cout<<"WORD: "<<word<<" posting_list size: "<<pl->size()<<endl;
		for(auto it = pl->begin(); it!= pl->end();++it){
			int v_i = *it;

			auto sit = soc_map.find(v_i);
			if(sit!=soc_map.end()){
				auto friendPair = sit->second;
				friends = friendPair.first;
				friendsSize = friendPair.second;
			}
			else{
				spos->getFriends(v_i, friends, friendsSize);
				soc_map.insert(make_pair(v_i,make_pair(friends,friendsSize)));
			}
			// cout<<"\t user v_i: "<<v_i<<" friendsize: "<<friendsSize<<" Friends: ";

			//find index greater than the id of the friend
			for(unsigned int j = lower_bound(friends, friends+friendsSize, v_i) - friends; j < friendsSize; j++){
				int f_id = friends[j];
				// cout<<f_id<<" ";
				auto pit = pl->find(f_id);
				if(pit!=pl->end()){
					if(word=="food"){
						FrConFood.emplace_back(v_i,f_id);
					}
					// res_point* localP = hit->second;
					++word_counter;

				}
			}
				// cout<<"\n";
		}

		if(word_counter>0){
			if(word_counter > best_score){
				topk.insert(tc_pair(word,word_counter));
			}
			if(topk.size() >= k){
				topk_iter = topk.begin();
				advance (topk_iter,k-1);
				tc_pair tc = *topk_iter;
				best_score = tc.getOT();
			}
		}
	}


	unsigned int z = 0;
	for(auto it = topk.begin(); it!= topk.end();++it){

		z++;if(z>k)break;
		if(z==topk.size()) break;
		tc_pair tc = *it;
		int idf = gpos->getIDF(tc.getWD());
		cout << z<<","<<tc.getWD()<<","<<idf<<","<<tc.getOT()<<endl;
		// if(tc.getWD() == "food"){
		// 	for(auto asd = FrConFood.begin(); asd != FrConFood.end(); ++asd){
		// 		pair<int,int> couple = *asd;
		// 		cout<<"<"<<couple.first<<", "<<couple.second<<"> "<<endl;
		// 	}
		// 	cout<<endl;
		// }
	}

	return heap;

}




