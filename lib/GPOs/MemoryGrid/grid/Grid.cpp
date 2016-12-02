#include "../../../headersMemory.h"

Grid::Grid (){

    locations = new unordered_map<int , Point*>();
	poi_locations = new unordered_map<int , Point*>();
    cout << "MIN_X = " << MIN_X << " MAX_X = " << MAX_X << " DELTA_X = " << DELTA_X << endl;
    cout << "MIN_Y = " << MIN_Y << " MAX_Y = " << MAX_Y << " DELTA_Y = " << DELTA_Y << endl;

	for (int i=0; i<X; i++){
        for (int j=0; j<Y; j++){
            Cell* c = new Cell;
            c->setDimensions(MIN_X+(i*DELTA_X), MIN_Y+(j*DELTA_Y), MIN_X+((i+1)*DELTA_X), MIN_Y+((j+1)*DELTA_Y));
            //            cout << MIN_X+(i*DELTA_X) << " , " <<  MIN_Y+(j*DELTA_Y) << " , " <<  MIN_X+((i+1)*DELTA_X) << " , " <<  MIN_Y+((j+1)*DELTA_Y) << endl;		
            c->setIndex(i, j);
			c->setType(CELL);
			// cout<<"Creating cell <"<< c->getIndexI()<<" , "<<c->getIndexJ()<<"> into grid | x1 "<<c->getX1()<<" y1 "<<c->getY1()<<" x2 "<<c->getX2()<<" y2 "<<c->getY2()<<" range is: ["<< ((c->getX1() - MIN_X)/DELTA_X)<< ", " << ((c->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((c->getX2() - MIN_X)/DELTA_X) << ", " << ((c->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
            table[i][j] = c;
			// table[i]->push_back(c);
        }
    }

	lastUserId = -1;
	lastPOIId = -1;
	lastCell = NULL;
	
	boundary_control = 0.00000000001;

}     

void Grid::setSPOs(SPOs* _spos){
	spos = _spos;
	if(spos == NULL)
		cout<<"FAILED TO SEND SPOS TO GPOS"<<endl;
}



//length is in number of cells contained
QuadTreeNode* Grid::buildNode(double x1, double y1, double x2, double y2, unsigned int level, unsigned int length, QuadTreeNode* parentNode){
	
	// cout<<"\n\t Building Node at level: "<<level<<" length: "<<length << "|  x1 "<<x1<<" y1 "<<y1<<" x2 "<<x2<<" y2 "<<y2<<" range is: ["<< ((x1 - MIN_X)/DELTA_X)<< ", " << ((y1 - MIN_Y)/DELTA_Y) << "] to [" << ((x2 - MIN_X)/DELTA_X) << ", " << ((y2 - MIN_Y)/DELTA_Y) << "]" << endl;
	//make local qtn
	QuadTreeNode* currentNode = new QuadTreeNode(x1,y1,x2,y2, level, length, parentNode);
	currentNode->setType(QTN);
	
	// int b_capacity = 1000; // can be multiple of length or level
	// int b_error_rate = 0.1;
	
	//instantiate scaling bloom filter of the currentNode
	// if (!(currentNode->bloom = new_scaling_bloom(b_capacity, b_error_rate, "garbage"))) {
        // fprintf(stderr, "ERROR: Could not create bloom filter\n");
    // }

	
	// set bloom filter parameters
	bloom_parameters parameters;
	 // How many elements roughly do we expect to insert?
	parameters.projected_element_count = 50000; // can be multiple of length or level
	// Maximum tolerable false positive probability? (0,1)
	parameters.false_positive_probability = 0.1; // 100 in 10000
	// Simple randomizer (optional)
	parameters.random_seed = 0xA5A5A5A5;
	if (!parameters)
	{
	   std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
	}
	parameters.compute_optimal_parameters();
	// Instantiate Bloom Filter
	currentNode->filter = new bloom_filter(parameters);
	
	// cout<<"Size of BLOOM FILTER is: "<<sizeof(currentNode->filter)<<" B"<<endl;
	
	//children
	int local_max_degree = 0;
	if(level == 1){
		currentNode->cellMembers = new vector<Cell*>();
	
		//get all the cells in the spatial extent
		vector<Cell*>* cells = getCells(x1,y1,x2,y2);
		for(auto itc = cells->begin(); itc != cells->end() ; ++itc){
			currentNode->cellMembers->push_back(*itc);
			
			// set their parent node to this node
			(*itc)->pParentNode = currentNode;
			
			//set number of occupants of currentNode
			currentNode->numOccupants += (*itc)->getCheckIns()->size();
			
			//set points of interest of currentNode
			currentNode->numPointsOfInterest += (*itc)->getPointsOfInterest()->size();
		}
		
		
		// for(auto itc = currentNode->cellMembers->begin(); itc != currentNode->cellMembers->end() ; ++itc){
			
		// }
		
		
		auto IDX = currentNode->idxImpact;
		auto P_IDX = currentNode->P_idxImpact;
		for(auto itc = currentNode->cellMembers->begin(); itc != currentNode->cellMembers->end() ; ++itc){
			if((*itc)->max_degree > local_max_degree){
				local_max_degree = (*itc)->max_degree;
			}
			
			//FOR USERS
			if(!(((*itc))->idxOfSet.empty())){
				//instead of keeping the max tf, keep the max impact for each term
				double local_max_impact = 0;
				unordered_map<string, double >* c_idxImpact = (*itc)->idxImpact;
				auto c_idxOfImpacts = (*itc)->idxOfImpacts;
				
				for(map<string, multiset<tf_pair, pair_descending_frequency>* >::iterator iter = ((*itc))->idxOfSet.begin(); iter != (*itc)->idxOfSet.end(); ++iter) {
					
					multiset<tf_pair, pair_descending_frequency>* pl = iter->second;
					multiset<my_pair, pair_comparator_descending>* impactSet = new multiset<my_pair, pair_comparator_descending>;	
					//iterate through whole posting list and find the user with max impact
					// alongside create maxImpact idx and posting list of impacts for the cells

					for(auto it_pl = pl->begin();it_pl!=pl->end();++it_pl){
						tf_pair tf_user = *it_pl;
						int u_id = tf_user.getId();
						int tf = tf_user.getTF();
						auto it_asd = ((*itc))->docLenMap.find(u_id);
						double local_impact = ( 1+ log10(tf) ) / sqrt(it_asd->second);
						// cout<<"inserting into impactSet user: "<<u_id <<" with tf = "<<tf_user.getTF()<<" doc length: "<<sqrt(it_asd->second)<<" and impact = "<<local_impact<<endl;
						impactSet->insert(my_pair(u_id,local_impact));
						if(local_impact > local_max_impact)
							local_max_impact = local_impact;
					}
					c_idxOfImpacts->insert(make_pair(iter->first,impactSet));
					//for each term keep maxImpact
					c_idxImpact->insert(make_pair(iter->first,local_max_impact));
					
					//update currentNode's max impact for term t
					auto it = IDX->find(iter->first);
					if(it != IDX->end()){
						if(local_max_impact > it->second){
							it->second = local_max_impact;
						}
					}
					else{
						IDX->insert(make_pair(iter->first,local_max_impact));
					}
				}
			}
			
			//FOR POIs
			
			if(!(((*itc))->P_idxOfSet.empty())){
				//bitwise & all bloom filters of the cells
				// cout<<"currentNode->filter->element_count() changed from: "<<currentNode->filter->element_count(); 
				if(((*itc))->filter!= NULL)
					(currentNode->filter)->setBitsFrom(((*itc))->filter);
				// cout<<" to "<<currentNode->filter->element_count()<<endl;
			
				//instead of keeping the max tf, keep the max impact for each term
				double local_max_impact = 0;
				unordered_map<string, double >* p_idxImpact = (*itc)->P_idxImpact;
				auto p_idxOfImpacts = (*itc)->P_idxOfImpacts;

				for(map<string, multiset<tf_pair, pair_descending_frequency>* >::iterator iter = ((*itc))->P_idxOfSet.begin(); iter != (*itc)->P_idxOfSet.end(); ++iter) {
					
					multiset<tf_pair, pair_descending_frequency>* pl = iter->second;
					multiset<my_pair, pair_comparator_descending>* impactSet = new multiset<my_pair, pair_comparator_descending>;	
					//iterate through whole posting list and find the user with max impact
					// alongside create maxImpact idx and posting list of impacts for the cells

					for(auto it_pl = pl->begin();it_pl!=pl->end();++it_pl){
						tf_pair tf_user = *it_pl;
						int poi_id = tf_user.getId();
						int tf = tf_user.getTF();
						auto it_asd = ((*itc))->P_docLenMap.find(poi_id);
						double local_impact = ( 1+ log10(tf) ) / sqrt(it_asd->second);
						// cout<<"inserting into impactSet user: "<<poi_id <<" with tf = "<<tf_user.getTF()<<" doc length: "<<sqrt(it_asd->second)<<" and impact = "<<local_impact<<endl;
						impactSet->insert(my_pair(poi_id,local_impact));
						if(local_impact > local_max_impact)
							local_max_impact = local_impact;
					}
					p_idxOfImpacts->insert(make_pair(iter->first,impactSet));
					//for each term keep maxImpact
					p_idxImpact->insert(make_pair(iter->first,local_max_impact));
					
					//update currentNode's max impact for term t
					auto it = P_IDX->find(iter->first);
					if(it != P_IDX->end()){
						if(local_max_impact > it->second){
							it->second = local_max_impact;
						}
					}
					else{
						P_IDX->insert(make_pair(iter->first,local_max_impact));
					}
				}
			}

			// cout<<"\t\t\t cell in L1 qtn is : <"<<(*itc)->getIndexI()<<","<<(*itc)->getIndexJ()<<"> with max degree"<< (*itc)->max_degree<<" num_of_users: "<<(*itc)->getCheckIns()->size() <<" filter_count:"<<((*itc))->filter->element_count() <<endl;
			// cout<<"\t\t\t cell in L1 qtn is : <"<<(*itc)->getIndexI()<<","<<(*itc)->getIndexJ()<<"> with max degree"<< (*itc)->max_degree<<" num_of_users: "<<(*itc)->getCheckIns()->size() <<" idxOfSet size: "<<((*itc))->P_idxOfSet.size()<<" idxOfImpacts size: "<<c_idxOfImpacts->size()<<" cell maxImpacts size "<<p_idxImpact->size()<<" filter_count:"<<((*itc))->filter->element_count() <<endl;
		
		}
		
		currentNode->max_degree = local_max_degree;
		// cout<<"\t\t Node at level 1 built with length "<<length<<" and max_degree: "<<local_max_degree<<endl;
	}
	else{
		currentNode->children = new vector<QuadTreeNode*>();
		//set their parent node to this node
		local_max_degree = 0;
		
		int qwer= 0;
		int l_length = round(length / (double)R_FAN);
		for(int xx = 0; xx < R_FAN; ++xx){
			for(int yy =0; yy < R_FAN; ++yy){
				//not correct - needs to be adjusted for height
				double x_offset = DELTA_X * l_length;
				double y_offset = DELTA_Y * l_length;
				// cout<<"\tCreating Children..."<<qwer<<endl;
				currentNode->children->push_back(buildNode(x1+(xx*x_offset), y1+(yy*y_offset), x1+((xx+1)*x_offset), y1+((yy+1)*y_offset),level-1, l_length, currentNode));
				qwer++;
			}
		}	
		
		auto IDX = currentNode->idxImpact;
		auto P_IDX = currentNode->P_idxImpact;
		
		//create bloom filter of current node

		// carry forward social information 
		for(auto itq = currentNode->children->begin(); itq != currentNode->children->end() ; ++itq){
		
		
			//set number of occupants of currentNode
			currentNode->numOccupants += (*itq)->numOccupants;
			
			//set points of interest of currentNode
			currentNode->numPointsOfInterest += (*itq)->numPointsOfInterest;
		
		
			if( (*itq)->max_degree > local_max_degree){
				local_max_degree = (*itq)->max_degree;
			}

			//FOR USERS
			if(!((*itq)->idxImpact->empty())){
				for(auto iter = (*itq)->idxImpact->begin(); iter != (*itq)->idxImpact->end(); ++iter) {
					string word = iter->first; // word in child index
					double local_impact = iter->second;
					auto it = IDX->find(word); // find word in current node index
					if(it != IDX->end()){		// if found
						//compare impact in current node vs child node.
						if(local_impact > it->second){
							it->second = local_impact;
						}
					}
					else{
						IDX->insert(make_pair(word,local_impact));
					}
				}
			}
			
			//FOR POIs
			if(!((*itq)->P_idxImpact->empty())){
			
				//bitwise & all filters in the lower level
				// cout<<"currentNode->filter->element_count() changed from: "<<currentNode->filter->element_count(); 
				if(((*itq))->filter!= NULL)
					(currentNode->filter)->setBitsFrom(((*itq))->filter);
				// cout<<" to "<<currentNode->filter->element_count()<<endl;

				for(auto iter = (*itq)->P_idxImpact->begin(); iter != (*itq)->P_idxImpact->end(); ++iter) {
					string word = iter->first; // word in child index
					double local_impact = iter->second;
					auto it = P_IDX->find(word); // find word in current node index
					if(it != P_IDX->end()){		// if found
						//compare impact in current node vs child node.
						if(local_impact > it->second){
							it->second = local_impact;
						}
					}
					else{
						P_IDX->insert(make_pair(word,local_impact));
					}
				}
			}
		}
		currentNode->max_degree = local_max_degree;
		//carry forward textual information
		// cout<<"\t Node at level "<<level<<" built with length "<<length<<" and max_degree: "<<currentNode->max_degree<<endl;
	
	}
	// cout<<"\t\t Setting max_degree of level "<<level<<" length "<<length<<" node to "<<local_max_degree<<endl;
	
	return currentNode;
}

/*
vector <QuadTreeNode*>* Grid::createQuadTree(int level){
	
	if(X != Y){
		cout<<"Quad tree needs same number of cells in the X and Y dimensions...exiting."<<endl;
		exit (EXIT_FAILURE);
	}
	//determine size of return vector based on length and level
	// at level 4, for X = 128, we have 8x8 matrix at the rootnode
	// at level 3, for X=Y=128, we get a 16x16 matrix at the rootnode
	int length = pow(2,level);
	
	cout<<"creating quad tree...levels: "<<level<<" length: "<<length<<endl;
	
	rootNode = new vector<QuadTreeNode*>();
	int counter = 0;
	for (int i=0; i<X/length; i++){
        for (int j=0; j<X/length; j++){
			// cout<<"root level node number:"<<counter<<": x1 "<< MIN_X+(i*length*DELTA_X)<<" y1 "<< MIN_Y+(j*length*DELTA_Y)<<" x2 " << MIN_X +((i+1)*length*DELTA_X)<<" y2 " <<MIN_Y +((j+1)*length*DELTA_Y)<<endl;;
			rootNode->push_back(buildNode(		MIN_X +(i*length*DELTA_X)		,MIN_Y +(j*length*DELTA_Y)			,MIN_X +((i+1)*length*DELTA_X)				,MIN_Y +((j+1)*length*DELTA_Y)		, level, length, NULL));
			counter++;
		}
    }	
	
	return rootNode;
}
*/
	
vector <QuadTreeNode*>* Grid::createQuadTree(){
	
	if(X != Y){
		cout<<"HG needs same number of cells in the X and Y dimensions...exiting."<<endl;
		exit (EXIT_FAILURE);
	}
	//determine size of return vector based on length and level
	// at level 4, for X = 128, we have 8x8 matrix at the rootnode
	// at level 3, for X=Y=128, we get a 16x16 matrix at the rootnode
	
	double fanout = pow(X*Y, 1.0/HG_HEIGHT);
	if (abs(fanout - (round(fanout))) > 0.000000001) {
		cout<<"HG VARIABLES ARE BAD."<<endl;
		exit (EXIT_FAILURE);
	}
	
	struct timeval start,end;
    double usec;
    
	cout<<"creating quad tree with Height: "<<HG_HEIGHT<<" Fanout: "<<HG_FANOUT<<" Granularity: "<<X<<" x "<<X<<endl;
	rootNode = new vector<QuadTreeNode*>();
	gettimeofday(&start, NULL);
	rootNode->push_back(buildNode(MIN_X,MIN_Y,MAX_X+DELTA_X,MAX_Y+DELTA_Y, HG_HEIGHT,X, NULL));
	gettimeofday(&end, NULL);
	usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
	cout<<"Quad Tree Construction Time: "<<usec/1000<<" ms"<<endl;
	return rootNode;
}

void Grid::deleteQuadTree(){

	for(auto it = rootNode->begin(); it!=rootNode->end();++it){
		delete *it;
    }	
	
}


Grid::~Grid (){
    for (int i=0; i<=X; i++){
        for (int j=0; j<=Y; j++){
            delete table[i][j];
        }
    }
    delete locations;
	deleteQuadTree();
}

vector<Cell*>* Grid::getCells(double x1, double y1, double x2, double y2){

	int q_x1 = (int)round(((x1 - MIN_X)/DELTA_X));
    int q_y1 = (int)round(((y1 - MIN_Y)/DELTA_Y));
	int q_x2 = (int)round(((x2 - MIN_X)/DELTA_X));
    int q_y2 = (int)round(((y2 - MIN_Y)/DELTA_Y));

	// cout<<"\t GIB CELLS: x1 "<<x1<<" y1 "<<y1<<" x2 "<<x2<<" y2 "<<y2<<" CELSS: ["<< ((x1 - MIN_X)/DELTA_X)<< ", " << ((y1 - MIN_Y)/DELTA_Y) << "] to [" << ((x2 - MIN_X)/DELTA_X) << ", " << ((y2 - MIN_Y)/DELTA_Y) << "]" << endl;
    // cout<<"\t Need cells in range[" << q_x1 << ", " << q_y1 << "] to [" << q_x2 << ", " << q_y2 << "]" << endl;
	vector<Cell*>* cells = new vector<Cell*>();
	
	if((q_x2 - q_x1) != 2 || (q_y2 - q_y1) != 2){
		// cout<<"BOUNDARY UNDEFNINED"<<endl;
	}

    if(q_x1 >=0 && q_x1 < X && q_y1 >=0 && q_y1 < Y){
		for(int i = q_x1; i < q_x2; ++i)
			for(int j = q_y1; j < q_y2; ++j){
				// cells->push_back(table[i][j]);
				cells->push_back(table[i][j]);
			}
		return cells;
	}
    else{
		cout<<"Range unbounded."<<endl;
		return NULL;
	}
}   

Cell* Grid::getCell(double x, double y){
    double q_x_raw = ((x - MIN_X)/(double)DELTA_X);
    double q_y_raw = ((y - MIN_Y)/(double)DELTA_Y);
	
	if(q_x_raw < 0 || q_y_raw < 0 ){
		return NULL;
	}
	int q_x = (int)q_x_raw;
	int q_y = (int)q_y_raw;
    // cout << "\t cell located: [" << q_x << ", " << q_y << "]" << endl;

    if(q_x >=0 && q_x < X && q_x >=0 && q_y < Y)
        return  table[q_x][q_y];
    else
        return NULL;
}   

Cell* Grid::getCell(int i, int j){

    if(i >= 0 && j >= 0 && i < X && j < Y)
        return table[i][j];
    else
        return NULL;

}


bool Grid::addToSIDX(const string& word, int occurrences, Point* user){
	
	if(user->getID() != lastUserId){
		lastCell = getCell(user->getX(), user->getY());
	}

    if(lastCell != NULL){
		lastCell->newSIDX(word, occurrences, user->getID());
        return true;
    }
    else{
        cout << "SIDX insert failed! (" << user->getX() <<", " << user->getY() << endl;
        return false;
    }
}


bool Grid::addCheckIn(Point* user){
    Cell * c = getCell(user->getX(), user->getY());
    if(c != NULL){
        c->newCheckIn(user, spos->getUserDegree(user->getID()));
        int id = user->getID();
        // cout << "Add checkin id = " << id ;
        locations->insert(pair<int, Point*>(id, user));
        // cout << "  locations size = " << locations->size() << endl;
        return true;
    }
    else{
        cout << "checkin failed! (" << user->getX() <<", " << user->getY() << endl;
        return false;
    }
} 

bool Grid::addPOIToSIDX(const string& word, int occurrences, Point* poi){
	
	if(poi->getID() != lastPOIId){
		lastCell = getCell(poi->getX(), poi->getY());
	}

    if(lastCell != NULL){
		lastCell->newPOIidxEntry(word, occurrences, poi->getID());
        return true;
    }
    else{
        cout << "POI SIDX insert failed! (" << poi->getX() <<", " << poi->getY() << endl;
        return false;
    }
}

bool Grid::addPOI(Point* poi){
    Cell * c = getCell(poi->getX(), poi->getY());
    if(c != NULL){
        c->newPOI(poi);
        int id = poi->getID();
        // cout << "Add POI id = " << id ;
        poi_locations->insert(pair<int, Point*>(id, poi));
        // cout << "  poi_locations size = " << poi_locations->size() << endl;
        return true;
    }
    else{
        cout << "poi insert failed! (" << poi->getX() <<", " << poi->getY() <<")"<< endl;
        return false;
    }
}

void Grid::newPOIHistoryEntry(int poi_id, int history_checkin){
	auto it = poi_locations->find(poi_id);
	if(it!=poi_locations->end()){
		Point* poi = it->second;
		Cell * c = getCell(poi->getX(), poi->getY());
		c->newPOIHistoryEntry(poi_id,history_checkin);;
		// cout<<"POI: "<< poi->getID()<<" history_checkin inserted with size "<<history_set->size()<<" History Map now contains: "<<c->P_historyMap.size()<<" elements"<<endl;
	}
	 else{
        cout << "history_checkin insert failed! "<< endl;
    }

}

bool Grid::setHistoryPOI(int poi_id, unordered_set<int>* history_set){
	auto it = poi_locations->find(poi_id);
	if(it!=poi_locations->end()){
		Point* poi = it->second;
		Cell * c = getCell(poi->getX(), poi->getY());
		c->P_historyMap.insert(make_pair(poi_id, history_set));
		// cout<<"POI: "<< poi->getID()<<" history_checkin set inserted with size "<<history_set->size()<<" History Map now contains: "<<c->P_historyMap.size()<<" elements"<<endl;
		return true;
	}
	else{
		cout << "POI NOT FOUND to insert history of checkins" << endl;
        return false;
	}
}


bool Grid::isUserInRange(double x, double y, double radius, int f_id){
	
    Cell* c = NULL;

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    // int count = 0;
    // int totalcells=0;
	
	bool flag = false;

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;

    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            // totalcells++;
            c = getCell(i, j);
            // count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
				if(c->isUserInCell(f_id)){
					flag = true; 
					break;
				}	
            }

        }
    }
	return flag;

    //    v.setStatistics(visits, count);
    //    cout << "total users = " << count << endl;
    //    cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;
}

void Grid::updateCheckIn(Point* p, double old_x, double old_y){

    //go to the previous cell and delete him
    //Point *oldP = getPoint(p->getID());
    //oldP->printDetails();
    Cell * c = getCell(old_x, old_y);
    if(c != NULL){
        c->deleteCheckIn(p);
    }
    //cout << "go to the previous cell and delete him finished " << endl;
    //the update of the hashtable has already be done in the gpos! this is only responsible for the grid
    //add him to the list of the new cell
    c = getCell(p->getX(), p->getY());
    if(c != NULL){
        c->newCheckIn(p, spos->getUserDegree(p->getID()));
    }
    //cout << "add him to the list of the new cell finished " << endl;
}


Point* Grid::getPoint(int id){

    unordered_map<int, Point*>::iterator it;
    it = locations->find(id);
    if(it != locations->end())
        return (*it).second;

    return NULL;
}

void Grid::getLocation(int id, Visitor& v){

    Point* tmp = getPoint(id);
    if(tmp!=NULL)
        v.setResult(tmp->getID(), 0, tmp->getX(), tmp->getY());
}

//computes the k next NN and saves them to the visitor

void Grid::getNextNN(IncrVisitor& v, int k){

    if(k > DATASET_SIZE)
        k = DATASET_SIZE;

    double x = v.getX();
    double y = v.getY();
    double best_dist = 9999999;
    priority_queue<Cell*, vector<Cell*>, Cell::ascending>* heap = v.getHeap();
    //list<Point*>* best_NN = v.getPointersToResults();
    int levels[4];

    levels[0] = v.getULevel();
    levels[1] = v.getRLevel();
    levels[2] = v.getDLevel();
    levels[3] = v.getLLevel();

    Cell* c;

    if(v.getResult()->size() >= DATASET_SIZE){
        v.finished = true;
        return;
    }

    if(heap->size() == 0){
        Cell* cq = getCell(x, y);
        cq->setMinDist(0.0);
        heap->push(cq);

        for(int i=0; i<4; i++){
            c = new Cell;
            getRectangle(i, 0, x, y, *c);
            if(c != NULL){
                heap->push(c);
                levels[i] = levels[i] + 1;
            }
        }
    }
    Cell* tmp = heap->top();
    int count = 0;

    while ( !heap->empty() &&  tmp->getMinDist() < best_dist && count < k && v.getResult()->size() < DATASET_SIZE) {

        heap->pop();

        if (tmp->getType() == CELL){                               //it is a cell
            list<Point*>* L = tmp->getCheckIns();
            list<Point*>::iterator it;
            Point* p = NULL;
            for(it=L->begin(); it != L->end(); ++it){
                p = *it;
                Cell * c = new Cell;
                c->setType(POINT);
                c->setID(p->getID());
                c->setDimensions(p->getX(), p->getY(), p->getX(), p->getY());
                c->computeMinDist(x, y);
                heap->push(c);
            }
        }
        else if(tmp->getType() == RECTANGLE){                      //it is a rectangle for a specific direction
            int direction = tmp->getDirection();
            double x1 = tmp->getX1();
            double y1 = tmp->getY1();
            double x2 = tmp->getX2();
            double y2 = tmp->getY2();

            if (direction == UP || direction == DOWN){
                for(double i = (x1+DELTA_X/2.0); i < x2; i+=DELTA_X){
                    Cell* cr = getCell(i, (y1+DELTA_Y/2.0));
                    if(cr != NULL){
                        cr->computeMinDist(x, y);
                        heap->push(cr);
                    }
                }
            }else if (direction == RIGHT || direction == LEFT){
                for(double j = (y1+DELTA_Y/2); j < y2; j+=DELTA_Y){
                    Cell* cr = getCell((x1+DELTA_X/2), j);
                    if(cr != NULL){
                        cr->computeMinDist(x, y);
                        heap->push(cr);
                    }
                }
            }else{
                cout << "Grid::getNextNN::Error, there is no such direction: " << direction << endl;
            }

            Cell* c = new Cell;
            getRectangle(direction, levels[direction], x, y, *c);
            heap->push(c);

            levels[direction] = levels[direction] + 1;
            delete tmp;
        }
        else{                                                                 // it is a point
            v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            v.setBestDist(best_dist);
            v.setLevels(levels[0], levels[1], levels[2], levels[3]);
            delete tmp;
            count++;
        }
        tmp = heap->top();

    }

}

//returns the rectangle for a direction and level
// query cell (i, j)
void Grid::getRectangle(int direction, int level, double x, double y, Cell& c){ 

    int i = (int)((x - MIN_X)/DELTA_X);
    int j = (int)((y - MIN_Y)/DELTA_Y);

    double x1, y1, x2, y2;

    if (direction == UP){

        x1 = MIN_X +(i - level)*DELTA_X;
        y1 = MIN_Y + (j + 1 + level)*DELTA_Y;
        x2 = MIN_X +(i + 2 + level)*DELTA_X;
        y2 = MIN_Y + (j + 2 + level)*DELTA_Y;
        
    }
    else if (direction == DOWN){

        x1 = MIN_X +(i - level-1)*DELTA_X;
        y1 = MIN_Y + (j - 1 - level)*DELTA_Y;
        x2 = MIN_X +(i + 1 + level)*DELTA_X;
        y2 = MIN_Y + (j - level)*DELTA_Y;
        
    }
    else if (direction == RIGHT){

        x1 = MIN_X +(i + level + 1)*DELTA_X;
        y1 = MIN_Y + (j - level - 1)*DELTA_Y;
        x2 = MIN_X +(i + 2 + level)*DELTA_X;
        y2 = MIN_Y + (j + level + 1)*DELTA_Y;

    }
    else if (direction == LEFT){

        x1 = MIN_X + (i - 1 - level)*DELTA_X;
        y1 = MIN_Y + (j - level)*DELTA_Y;
        x2 = MIN_X + (i - level)*DELTA_X;
        y2 = MIN_Y + (j + level + 2)*DELTA_Y;

    }
    else
        cout << "Error, there is no such a direction" << endl;


	c.setDimensions(x1, y1, x2, y2);
	c.setDirection(direction);
	c.computeMinDist(x, y);
	c.setType(RECTANGLE);

}         

//return the cumulative percentile histogram CH_DIST
map<int, double>* Grid::createCumulativeHistogram(double x, double y, int numOfBins){
	
	map<int, double>* CH_DIST = new map<int, double>();
	
	// skip every R_FAN cells 
	for (int i=0; i<X; i+=R_FAN){
		for (int j=0; j<Y; j+=R_FAN){
			Cell* c =  table[i][j];
			QuadTreeNode* pParentNode = c->pParentNode; 
			pParentNode->computeMinDist(x,y);
			double dist_to_node = pParentNode->getMinDist() + DELTA_XY;
			int bin = floor(dist_to_node);
			int numOccupants = pParentNode->numOccupants;
			
			auto it = CH_DIST->find(bin);
			if(it != CH_DIST->end()){
				it->second = it->second + numOccupants;
			}
			else{
				CH_DIST->insert(make_pair(bin,numOccupants));
			}
		}
	}
	
	//incrementally cumulate number of users
	auto prev_it = CH_DIST->begin();
	auto it = CH_DIST->begin(); ++it;
	while(it != CH_DIST->end()){
		it->second = (prev_it->second + it->second)*CONV_PERCENTILE;
		++prev_it;
		++it;
	}
	
	return CH_DIST;
	
}


list<Cell*>* Grid::getIntersectedCellsWithRectangle(double x1, double y1, double x2, double y2){

    list <Cell*> *result = new list<Cell*>;

    if(x1 < MIN_X)
        x1 = MIN_X;
    else if(x1 > MAX_X)
        x1 = MAX_X;

    if(x2 < MIN_X)
        x2 = MIN_X;
    else if(x2 > MAX_X)
        x2 = MAX_X;

    if(y1 < MIN_Y)
        y1 = MIN_Y;
    else if(y1 > MAX_Y)
        y1 = MAX_Y;

    if(y2 < MIN_Y)
        y2 = MIN_Y;
    else if(y2 > MAX_Y)
        y2 = MAX_Y;

    //cout << "x1 = " << x1 << "\t y1 = " << y1 << "\t x2 = " << x2 << "\t y2 = " << y2 << endl;

    int x_start = (int)((x1 - MIN_X)/DELTA_X);
    int x_end = (int)((x2 - MIN_X)/DELTA_X);
    int y_start = (int)((y1 - MIN_Y)/DELTA_Y);
    int y_end = (int)((y2 - MIN_Y)/DELTA_Y);

    for(int x = x_start; x <= x_end; x++){
        for(int y = y_start; y <= y_end; y++){
            Cell *c = getCell(x, y);
            //cout << "intersected cell: (" << x << ", " << y << ") => " << "x1 = " << c->getX1() << "\t y1 = " << c->getY1() << "\t x2 = " << c->getX2() << "\t y2 = " << c->getY2() << endl;
            result->push_front(c);
        }
    }

    return result;
} 

//visitor class outdated. please revise using other knn approaches
void Grid::getkNN(Visitor& v, int k){

    if(k > DATASET_SIZE)
        k = DATASET_SIZE;

    double x = v.getX();
    double y = v.getY();
    double best_dist = 9999999;
    //Point* inn = NULL;
    priority_queue<Cell*, vector<Cell*>, Cell::ascending>* heap = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>;
    //list<Point*>* best_NN = v.getPointersToResults();
    int levels[] = {0, 0, 0, 0};

    /*
    if(v.getResult()->size() >= DATASET_SIZE){
        v.finished == true;
        return;
    }
*/

    Cell* cq = getCell(x, y);
    cq->setMinDist(0.0);
    heap->push(cq);

    Cell* c;
    for(int i=0; i<4; i++){
        c = new Cell;
        getRectangle(i, 0, x, y, *c);
        if(c != NULL){
            heap->push(c);
            levels[i] = levels[i] + 1;
        }
    }

    Cell* tmp = heap->top();
    int count = 0;

    while ( !heap->empty() &&  tmp->getMinDist() < best_dist && count < k) {

        heap->pop();

        if (tmp->getType() == CELL){                               //it is a cell
            list<Point*>* L = tmp->getCheckIns();
            list<Point*>::iterator it;
            Point* p = NULL;
            for(it=L->begin(); it != L->end(); ++it){
                p = *it;
                Cell * c = new Cell;
                c->setType(POINT);
                c->setID(p->getID());
                c->setDimensions(p->getX(), p->getY(), p->getX(), p->getY());
                c->computeMinDist(x, y);
                heap->push(c);
            }
        }
        else if(tmp->getType() == RECTANGLE){                      //it is a rectangle for a specific direction
            int direction = tmp->getDirection();
            double x1 = tmp->getX1();
            double y1 = tmp->getY1();
            double x2 = tmp->getX2();
            double y2 = tmp->getY2();

            if (direction == UP || direction == DOWN){
                for(double i = (x1+DELTA_X/2.0); i < x2; i+=DELTA_X){
                    Cell* cr = getCell(i, (y1+DELTA_Y/2.0));
                    if(cr != NULL){
                        cr->computeMinDist(x, y);
                        heap->push(cr);
                    }
                }
            }else if (direction == RIGHT || direction == LEFT){
                for(double j = (y1+DELTA_Y/2); j < y2; j+=DELTA_Y){
                    Cell* cr = getCell((x1+DELTA_X/2), j);
                    if(cr != NULL){
                        cr->computeMinDist(x, y);
                        heap->push(cr);
                    }
                }
            }else{
                cout << "Grid::getkNN::Error, there is no such direction: " << direction << endl;
            }

            Cell* c = new Cell;
            getRectangle(direction, levels[direction], x, y, *c);
            heap->push(c);

            levels[direction] = levels[direction] + 1;
            delete tmp;
        }
        else{                                                                 // it is a point
            v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            v.setBestDist(best_dist);
            count++;
            delete tmp;
        }

        tmp = heap->top();

    }
    // delete the rest heap
    while(!heap->empty()) {
        Cell* c = heap->top();
        if(c->getType() != CELL)
            delete c;
        heap->pop();
    }
    delete heap;

}


res_point* Grid::getNextNearestUser(double x, double y, int signal){

	if(signal == 0){ //start execution - initialize levels and heap
		
		//initialize class variables
		nnu_heap = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>;
		for(int i = 0; i <4 ; ++i){
			nnu_levels[i] = 0;
		}
		nnu_count = 0;
		
		//load into heap
		Cell* cq = getCell(x, y);
		cq->setMinDist(0.0);
		nnu_heap->push(cq);

		Cell* c;
		for(int i=0; i<4; i++){
			c = new Cell;
			getRectangle(i, 0, x, y, *c);
			if(c != NULL){
				nnu_heap->push(c);
				nnu_levels[i] = nnu_levels[i] + 1;
			}
		}
	}
	
	if(signal == 1){	//use old heap and levels and continue loop
		Cell* cq = getCell(x, y);
		int q_indexI = cq->getIndexI();
		int q_indexJ = cq->getIndexJ();

		while ( !nnu_heap->empty() && nnu_count < DATASET_SIZE) {
			Cell* tmp = nnu_heap->top();
			nnu_heap->pop();
			if (tmp->getType() == CELL){                               //it is a cell
				list<Point*>* L = tmp->getCheckIns();
				list<Point*>::iterator it;
				Point* p = NULL;

				for(it=L->begin(); it != L->end(); ++it){
					p = *it;
					Cell * c = new Cell;
					c->setType(POINT);
					c->setID(p->getID());
					c->setDimensions(p->getX(), p->getY(), p->getX(), p->getY());
					c->computeMinDist(x, y);
					nnu_heap->push(c);
				}
			}
			else if(tmp->getType() == RECTANGLE){                      //it is a rectangle for a specific direction
				int direction = tmp->getDirection();
				double x1 = tmp->getX1();
				double y1 = tmp->getY1();
				double x2 = tmp->getX2();
				double y2 = tmp->getY2();

				if (direction == UP || direction == DOWN){
					for(double i = (x1+DELTA_X/2.0); i < x2; i+=DELTA_X){
						Cell* cr = getCell(i, (y1+DELTA_Y/2.0));
						if(cr != NULL){
							cr->computeMinDist(x, y);
							nnu_heap->push(cr);
						}
					}
				}else if (direction == RIGHT || direction == LEFT){
					for(double j = (y1+DELTA_Y/2); j < y2; j+=DELTA_Y){
						Cell* cr = getCell((x1+DELTA_X/2), j);
						if(cr != NULL){
							cr->computeMinDist(x, y);
							nnu_heap->push(cr);
						}
					}
				}else{
					cout << "Grid::getkNN::Error, there is no such direction: " << direction << endl;
				}
				
				bool isEnd = false;
				if (direction == UP && nnu_levels[direction] > Y - q_indexJ) {
					isEnd = true;
				}
				else if(direction == DOWN && nnu_levels[direction] > q_indexJ) {
					isEnd = true;
				}
				else if (direction == RIGHT && nnu_levels[direction] > X - q_indexI) {
					isEnd = true;
				}
				else if (direction == LEFT && nnu_levels[direction] > q_indexI) {
					isEnd = true;
				}
				if(!isEnd){
					Cell* c = new Cell;
					getRectangle(direction, nnu_levels[direction], x, y, *c);
					if(c!=NULL){
						nnu_heap->push(c);
						nnu_levels[direction] = nnu_levels[direction] + 1;
					}
					delete tmp;
				}
			}
			else{                                                                 // it is a point
				res_point* rp = new res_point();
				rp->id = tmp->getID();
				rp->x = tmp->getX1();
				rp->y = tmp->getY1();
				rp->dist = tmp->getMinDist();
				nnu_count++;
				delete tmp;
				return rp;
			}
		}
	}
	else if(signal == 2){	//clear out variables and delete heap
		 // delete the rest heap
		while(!nnu_heap->empty()) {
			Cell* c = nnu_heap->top();
			if(c->getType() != CELL)
				delete c;
			nnu_heap->pop();
		}
		delete nnu_heap;
	}
	
	return NULL;
}


vector<res_point*>* Grid::getkNN(double x, double y, int k){

    if(k > DATASET_SIZE)
        k = DATASET_SIZE;

    vector<res_point*>* result = new vector<res_point*>();
    res_point* rp;
    priority_queue<Cell*, vector<Cell*>, Cell::ascending>* heap = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>;
    int levels[] = {0, 0, 0, 0};

    Cell* cq = getCell(x, y);
	int q_indexI = cq->getIndexI();
	int q_indexJ = cq->getIndexJ();
    cq->setMinDist(0.0);
    heap->push(cq);

    Cell* c;
    for(int i=0; i<4; i++){
        c = new Cell;
        getRectangle(i, 0, x, y, *c);
		if(c!=NULL) {
            heap->push(c);
            levels[i] = levels[i] + 1;
        }
    }

    Cell* tmp = heap->top();
    int count = 0;

    while ( !heap->empty() && count < k) {
			
		// if(tmp->getID() == 11588){
			// cout<<"FOUND USER 11588 with getType = " << tmp->getType() <<endl;
		// }

        heap->pop();

        if (tmp->getType() == CELL){                               //it is a cell
            list<Point*>* L = tmp->getCheckIns();
            list<Point*>::iterator it;
            Point* p = NULL;
			// cout<<"points in cell number" <<tmp->getID()<<" are "<< L->size()<<endl;
			
            for(it=L->begin(); it != L->end(); ++it){
                p = *it;
                Cell * c = new Cell;
                c->setType(POINT);
                c->setID(p->getID());
				
                c->setDimensions(p->getX(), p->getY(), p->getX(), p->getY());
                c->computeMinDist(x, y);
                heap->push(c);
            }
        }
        else if(tmp->getType() == RECTANGLE){                      //it is a rectangle for a specific direction
            int direction = tmp->getDirection();
            double x1 = tmp->getX1();
            double y1 = tmp->getY1();
            double x2 = tmp->getX2();
            double y2 = tmp->getY2();

            if (direction == UP || direction == DOWN){
                for(double i = (x1+DELTA_X/2.0); i < x2; i+=DELTA_X){
                    Cell* cr = getCell(i, (y1+DELTA_Y/2.0));
                    if(cr != NULL){
                        cr->computeMinDist(x, y);
                        heap->push(cr);
                    }
                }
            }else if (direction == RIGHT || direction == LEFT){
                for(double j = (y1+DELTA_Y/2); j < y2; j+=DELTA_Y){
                    Cell* cr = getCell((x1+DELTA_X/2), j);
                    if(cr != NULL){
                        cr->computeMinDist(x, y);
                        heap->push(cr);
                    }
                }
            }else{
                cout << "Grid::getkNN::Error, there is no such direction: " << direction << endl;
            }
						bool isEnd = false;
			if (direction == UP && levels[direction] > Y - q_indexJ) {
				isEnd = true;
			}
			else if(direction == DOWN && levels[direction] > q_indexJ) {
				isEnd = true;
			}
			else if (direction == RIGHT && levels[direction] > X - q_indexI) {
				isEnd = true;
			}
			else if (direction == LEFT && levels[direction] > q_indexI) {
				isEnd = true;
			}
			if(!isEnd){
				Cell* c = new Cell;
				getRectangle(direction, levels[direction], x, y, *c);
				if(c!=NULL){
					heap->push(c);
					levels[direction] = levels[direction] + 1;
				}
				
				delete tmp;
			}

        }
        else{                                                                 // it is a point
            //v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            //v.setBestDist(best_dist);

            rp = new res_point();
            rp->id = tmp->getID();
            rp->x = tmp->getX1();
            rp->y = tmp->getY1();
            rp->dist = tmp->getMinDist();

            //			cout << "new result = " << rp->id << endl;

            result->push_back(rp);
            count++;
            delete tmp;
        }

        tmp = heap->top();

    }
    // delete the rest heap
    while(!heap->empty()) {
        Cell* c = heap->top();
        if(c->getType() != CELL)
            delete c;
        heap->pop();
    }
    delete heap;

    //	cout << "getKNN end size = " << result->size() << endl;

    return result;
}

//
res_point* Grid::getNextNearestNeighbourWithTerm(double x, double y, int signal, vector<string>* terms){

	if(signal == 0){ //start execution - initialize levels and heap
		// cout<<"start execution - initialize levels and heap"<<endl;
		//initialize class variables
		nnut_heap = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>;
		for(int i = 0; i <4 ; ++i){
			nnut_levels[i] = 0;
		}
		nnut_count = 0;
		
		//load into heap
		Cell* cq = getCell(x, y);
		for(auto it = terms->begin(); it != terms->end(); ++it){
			// cout<<"\t QUERY CELL | <"<<cq->getIndexI()<<" , "<<cq->getIndexJ()<<">  x1 "<<cq->getX1()<<" y1 "<<cq->getY1()<<" x2 "<<cq->getX2()<<" y2 "<<cq->getY2()<<" range covers is: ["<< ((cq->getX1() - MIN_X)/DELTA_X)<< ", " << ((cq->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cq->getX2() - MIN_X)/DELTA_X) << ", " << ((cq->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< cq->getMinDist() << endl;
			if(cq->idxOfSet.find(*it) != cq->idxOfSet.end()){
				cq->setMinDist(0.0);
				nnut_heap->push(cq);
				break;
			}
		}

		Cell* c;
		for(int i=0; i<4; i++){
			c = new Cell;
			getRectangle(i, 0, x, y, *c);
			if(c!=NULL) {
				nnut_heap->push(c);
				nnut_levels[i] = nnut_levels[i] + 1;
			}
		}
	}
	
	if(signal == 1){	//use old heap and levels and continue loop
		// cout<<"use old heap and levels and continue loop"<<endl;
		Cell* cq = getCell(x, y);
		int q_indexI = cq->getIndexI();
		int q_indexJ = cq->getIndexJ();

		while ( !nnut_heap->empty() && nnut_count < DATASET_SIZE) {
			Cell* tmp = nnut_heap->top();
			nnut_heap->pop();
			if (tmp->getType() == CELL){                               //it is a cell
				// cout<<"Popping CELL | <"<<tmp->getIndexI()<<" , "<<tmp->getIndexJ()<<">  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< tmp->getMinDist() << endl;
				unordered_set<int> seen_users;
				for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
					auto pl_iter = tmp->idxOfSet.find(*it); 		// find posting list
					if(pl_iter != tmp->idxOfSet.end()){
						multiset<tf_pair, pair_descending_frequency>* pl = pl_iter->second;
						if(pl->size() > 0 ){
							for(auto set_iter = pl->begin(); set_iter != pl->end();++set_iter){
								tf_pair tf_user = *set_iter;
								int u_id = tf_user.getId();
								
								if(seen_users.find(u_id) == seen_users.end()){
									seen_users.insert(u_id);
									Cell * c = new Cell;
									auto l_it = locations->find(u_id);
									if(l_it == locations->end())
										cout<<"LOCATION NOT FOUND FOR USER: "<<u_id<<"\n";
									Point* p = l_it->second;
									c->setType(POINT);
									c->setID(p->getID());
									c->setDimensions(p->getX(), p->getY(), p->getX(), p->getY());
									c->computeMinDist(x, y);
									nnut_heap->push(c);
								}
							}
						}
					}
				}
			}
			else if(tmp->getType() == RECTANGLE){                      //it is a rectangle for a specific direction
				int direction = tmp->getDirection();
				double x1 = tmp->getX1();
				double y1 = tmp->getY1();
				double x2 = tmp->getX2();
				double y2 = tmp->getY2();
				// cout<<"Popping RECTANGLE type "<<tmp->getType()<<" Direction = "<< tmp->getDirection()<<" at level " <<nnut_levels[tmp->getDirection()]<< " |  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< tmp->getMinDist() << endl;
				if (direction == UP || direction == DOWN){
					for(double i = (x1+DELTA_X/2.0); i < x2; i+=DELTA_X){
						Cell* cr = getCell(i, (y1+DELTA_Y/2.0));
									
						for(auto it = terms->begin(); it != terms->end(); ++it){
							if(cr != NULL){
								if(cr->idxOfSet.find(*it) != cr->idxOfSet.end()){	
									// cout<<"\t INSERTING CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
									cr->computeMinDist(x, y);
									nnut_heap->push(cr);
									break;
								}
								// cout<<"\t inserting CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< cr->getMinDist() << endl;
							}
						}
					}
				}else if (direction == RIGHT || direction == LEFT){
					for(double j = (y1+DELTA_Y/2); j < y2; j+=DELTA_Y){
						Cell* cr = getCell((x1+DELTA_X/2), j);
						for(auto it = terms->begin(); it != terms->end(); ++it){
							if(cr != NULL){
								if(cr->idxOfSet.find(*it)!=cr->idxOfSet.end()){
									// cout<<"\t INSERTING CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
									cr->computeMinDist(x, y);
									nnut_heap->push(cr);
									break;
								}
								// cout<<"\t inserting CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< cr->getMinDist() << endl;
							}
						}
						
					}
				}else{
					cout << "Grid::getkNN::Error, there is no such direction: " << direction << endl;
				}
				
				bool isEnd = false;
				if (direction == UP && nnut_levels[direction] > Y - q_indexJ) {
					isEnd = true;
				}
				else if(direction == DOWN && nnut_levels[direction] > q_indexJ) {
					isEnd = true;
				}
				else if (direction == RIGHT && nnut_levels[direction] > X - q_indexI) {
					isEnd = true;
				}
				else if (direction == LEFT && nnut_levels[direction] > q_indexI) {
					isEnd = true;
				}
				if(!isEnd){
					Cell* c = new Cell;
					getRectangle(direction, nnut_levels[direction], x, y, *c);
					if(c!=NULL){
						nnut_heap->push(c);
						nnut_levels[direction] = nnut_levels[direction] + 1;
					}
					delete tmp;
				}
			}
			else{                                                                 // it is a point
				//v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
				//v.setBestDist(best_dist);

				res_point * rp = new res_point();
				rp->id = tmp->getID();
				rp->x = tmp->getX1();
				rp->y = tmp->getY1();
				rp->dist = tmp->getMinDist();
				nnut_count++;
				delete tmp;
				return rp;
			}
		}
	}
	else if(signal == 2){	//clear out variables and delete heap
		// cout<<"clear out variables and delete heap"<<endl;
		while(!nnut_heap->empty()) {
			Cell* c = nnut_heap->top();
			if(c->getType() != CELL)
				delete c;
			nnut_heap->pop();
		}
		delete nnut_heap;
	}
	
	return NULL;
}


//
vector<res_point*>* Grid::getkNNfromIDX(double x, double y, int k, vector<string>* terms){

    if(k > DATASET_SIZE)
        k = DATASET_SIZE;

    vector<res_point*>* result = new vector<res_point*>();
    res_point* rp;
    priority_queue<Cell*, vector<Cell*>, Cell::ascending>* heap = new priority_queue<Cell*, vector<Cell*>, Cell::ascending>;

    int levels[] = {0, 0, 0, 0};

    Cell* cq = getCell(x, y);
	int q_indexI = cq->getIndexI();
	int q_indexJ = cq->getIndexJ();

		
	for(auto it = terms->begin(); it != terms->end(); ++it){
	// cout<<"\t QUERY CELL | <"<<cq->getIndexI()<<" , "<<cq->getIndexJ()<<">  x1 "<<cq->getX1()<<" y1 "<<cq->getY1()<<" x2 "<<cq->getX2()<<" y2 "<<cq->getY2()<<" range covers is: ["<< ((cq->getX1() - MIN_X)/DELTA_X)<< ", " << ((cq->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cq->getX2() - MIN_X)/DELTA_X) << ", " << ((cq->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< cq->getMinDist() << endl;
		if(cq->idxOfSet.find(*it) != cq->idxOfSet.end()){
			cq->setMinDist(0.0);
			heap->push(cq);
			break;
		}
	}

    Cell* c;
    for(int i=0; i<4; i++){
        c = new Cell;
        getRectangle(i, 0, x, y, *c);
		if(c != NULL){
            heap->push(c);
            levels[i] = levels[i] + 1;
        }
    }

    Cell* tmp = heap->top();
    int count = 0;

    while ( !heap->empty() && count < k) {
        heap->pop();
		
        if (tmp->getType() == CELL){                               //it is a cell
			// cout<<"Popping CELL | <"<<tmp->getIndexI()<<" , "<<tmp->getIndexJ()<<">  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< tmp->getMinDist() << endl;
			unordered_set<int> seen_users;
			for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
				auto pl_iter = tmp->idxOfSet.find(*it); 		// find posting list
				if(pl_iter != tmp->idxOfSet.end()){
					multiset<tf_pair, pair_descending_frequency>* pl = pl_iter->second;
					if(pl->size() > 0 ){
						for(auto set_iter = pl->begin(); set_iter != pl->end();++set_iter){
							tf_pair tf_user = *set_iter;
							int u_id = tf_user.getId();
							
							if(seen_users.find(u_id) == seen_users.end()){
								seen_users.insert(u_id);
								Cell * c = new Cell;
								auto l_it = locations->find(u_id);
								if(l_it == locations->end())
									cout<<"LOCATION NOT FOUND FOR USER: "<<u_id<<"\n";
								Point* p = l_it->second;
								c->setType(POINT);
								c->setID(p->getID());
								c->setDimensions(p->getX(), p->getY(), p->getX(), p->getY());
								c->computeMinDist(x, y);
								heap->push(c);
							}
						}
					}
				}
			}
        }
        else if(tmp->getType() == RECTANGLE){                      //it is a rectangle for a specific direction
            int direction = tmp->getDirection();
            double x1 = tmp->getX1();
            double y1 = tmp->getY1();
            double x2 = tmp->getX2();
            double y2 = tmp->getY2();
			// cout<<"Popping RECTANGLE type "<<tmp->getType()<<" size of result = "<<result->size()<<" Direction = "<< tmp->getDirection()<<" at level " <<levels[tmp->getDirection()]<< " |  x1 "<<tmp->getX1()<<" y1 "<<tmp->getY1()<<" x2 "<<tmp->getX2()<<" y2 "<<tmp->getY2()<<" range covers is: ["<< ((tmp->getX1() - MIN_X)/DELTA_X)<< ", " << ((tmp->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((tmp->getX2() - MIN_X)/DELTA_X) << ", " << ((tmp->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< tmp->getMinDist() << endl;
            if (direction == UP || direction == DOWN){
                for(double i = (x1+DELTA_X/2.0); i < x2; i+=DELTA_X){
                    Cell* cr = getCell(i, (y1+DELTA_Y/2.0));
								
                    for(auto it = terms->begin(); it != terms->end(); ++it){
						if(cr != NULL){
							if(cr->idxOfSet.find(*it) != cr->idxOfSet.end()){	
								// cout<<"\t INSERTING CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
								cr->computeMinDist(x, y);
								heap->push(cr);
								break;
							}
							// cout<<"\t inserting CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< cr->getMinDist() << endl;
						}
					}
                }
            }else if (direction == RIGHT || direction == LEFT){
                for(double j = (y1+DELTA_Y/2); j < y2; j+=DELTA_Y){
                    Cell* cr = getCell((x1+DELTA_X/2), j);
					for(auto it = terms->begin(); it != terms->end(); ++it){
						if(cr != NULL){
							if(cr->idxOfSet.find(*it)!=cr->idxOfSet.end()){
								// cout<<"\t INSERTING CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
								cr->computeMinDist(x, y);
								heap->push(cr);
								break;
							}
							// cout<<"\t inserting CELL <"<< cr->getIndexI()<<" , "<<cr->getIndexJ()<<"> into heap | x1 "<<cr->getX1()<<" y1 "<<cr->getY1()<<" x2 "<<cr->getX2()<<" y2 "<<cr->getY2()<<" range is: ["<< ((cr->getX1() - MIN_X)/DELTA_X)<< ", " << ((cr->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((cr->getX2() - MIN_X)/DELTA_X) << ", " << ((cr->getY2() - MIN_Y)/DELTA_Y) << "] minDist = "<< cr->getMinDist() << endl;
						}
					}
					
                }
            }else{
                cout << "Grid::getkNN::Error, there is no such direction: " << direction << endl;
            }
			
			bool isEnd = false;
			if (direction == UP && levels[direction] > Y - q_indexJ) {
				isEnd = true;
			}
			else if(direction == DOWN && levels[direction] > q_indexJ) {
				isEnd = true;
			}
			else if (direction == RIGHT && levels[direction] > X - q_indexI) {
				isEnd = true;
			}
			else if (direction == LEFT && levels[direction] > q_indexI) {
				isEnd = true;
			}
			if(!isEnd){
				Cell* c = new Cell;
				getRectangle(direction, levels[direction], x, y, *c);
				heap->push(c);
				
				levels[direction] = levels[direction] + 1;
				delete tmp;
			}
            
        }
        else{                                                                 // it is a point
            //v.setResult(tmp->getID(), tmp->getMinDist(), tmp->getX1(), tmp->getY1());
            //v.setBestDist(best_dist);

            rp = new res_point();
            rp->id = tmp->getID();
            rp->x = tmp->getX1();
            rp->y = tmp->getY1();
            rp->dist = tmp->getMinDist();

            			// cout << "new result = " << rp->id << endl;
            result->push_back(rp);
            count++;
            delete tmp;
        }

        tmp = heap->top();

    }
    // delete the rest heap
    while(!heap->empty()) {
        Cell* c = heap->top();
        if(c->getType() != CELL)
            delete c;
        heap->pop();
    }
    delete heap;

    //	cout << "getKNN end size = " << result->size() << endl;

	// cout<<"------------------------------------------------------------------------------------------------------------"<<endl;
	// for (int i=0; i<X; i++){
        // for (int j=0; j<Y; j++){
            // Cell* c = table[i][j];
			// cout<<"Printing cell <"<< c->getIndexI()<<" , "<<c->getIndexJ()<<"> into grid | x1 "<<c->getX1()<<" y1 "<<c->getY1()<<" x2 "<<c->getX2()<<" y2 "<<c->getY2()<<" range is: ["<< ((c->getX1() - MIN_X)/DELTA_X)<< ", " << ((c->getY1() - MIN_Y)/DELTA_Y) << "] to [" << ((c->getX2() - MIN_X)/DELTA_X) << ", " << ((c->getY2() - MIN_Y)/DELTA_Y) << "]" << endl;
        // }
    // }
	


    return result;
}



// 1. get the cells which are in the circle's MBR.
// 2. for each one of these, check if it intersects the circle,
// 3. if yes, get the checkins that are in the circle. 
void Grid::getRange(double x, double y, double radius, Visitor& v){  

    Point* p = NULL;
    Cell* c = NULL;

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    //cout << "x_start = " << x_start << " x_end = " << x_end << " y_start = " << y_start << " y_end = " << y_end << endl;

    int count = 0;
    int visits = 0;

    for(int i = x_start; i <= x_end && x_end < X; i++){
        for(int j = y_start; j <= y_end && y_end < Y; j++){
            c = getCell(i, j);
            count++;
            visits++;
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
                list<Point*>* L = c->getCheckIns();
                list<Point*>::iterator it;
                for(it=L->begin(); it != L->end(); ++it){
                    p = *it;
                    count++;
                    if(p->computeMinDist(x, y) <= radius)
                        v.setResult(p->getID(), p->getMinDist(), p->getX(), p->getY());
                }
            }
        }

    }
    v.setStatistics(visits, count);


}


// 1. get the cells which are in the circle's MBR.
// 2. for each one of these, check if it intersects the circle,
// 3. if yes, get the checkins that are in the circle. 
vector<res_point*>* Grid::getRange(double x, double y, double radius){  

    //    Cell* qPoint = NULL;
    //    qPoint=getCell(x,y);
    //    cout<<"query point is in cell: ("<<qPoint->getIndexI()<<" , "<<qPoint->getIndexJ()<<")"<<endl;

    Point* p = NULL;
    Cell* c = NULL;

    vector<res_point*>* result = new vector<res_point*>();

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    int count = 0;
    int visits = 0;
    int totalcells=0;

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;

    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            totalcells++;
            c = getCell(i, j);
            count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
                visits++;
                list<Point*>* L = c->getCheckIns();
                list<Point*>::iterator it;
                for(it=L->begin(); it != L->end(); ++it){
                    p = *it;
                    //count++;
                    if(p->computeMinDist(x, y) <= radius){
                        res_point* rp = new res_point();
                        rp->id = p->getID();
                        rp->x = p->getX();
                        rp->y = p->getY();
                        rp->dist = p->getMinDist();
                        result->push_back(rp);
                    }
                }
            }

        }
    }

    //    v.setStatistics(visits, count);
    //    cout << "total users = " << count << endl;
    //    cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;


    return result;
}

set<res_point*, res_point_ascending_dist>* Grid::getRangeWithIDX(double x, double y, double radius, vector<string> *terms, vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* _keywordVEC){  

    //    Cell* qPoint = NULL;
    //    qPoint=getCell(x,y);
    //    cout<<"query point is in cell: ("<<qPoint->getIndexI()<<" , "<<qPoint->getIndexJ()<<")"<<endl;

    Point* p = NULL;
    Cell* c = NULL;

    set<res_point*, res_point_ascending_dist>* result = new set<res_point*, res_point_ascending_dist>();

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    int count = 0;
    int visits = 0;
    int totalcells=0;

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;

    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            totalcells++;
            c = getCell(i, j);
            count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
				int qwe = 0;
				for(std::vector<string>::iterator it = terms->begin(); it != terms->end(); ++it) {
					auto pl = c->lookupWord(*it);
					if(pl != NULL)
						(_keywordVEC->at(qwe))->push_back(pl);
					++qwe;
				}
				//add to vector of vectors of posting lists
			
			
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
                visits++;
                list<Point*>* L = c->getCheckIns();
                list<Point*>::iterator it;
                for(it=L->begin(); it != L->end(); ++it){
                    p = *it;
                    //count++;
                    if(p->computeMinDist(x, y) <= radius){
                        res_point* rp = new res_point();
                        rp->id = p->getID();
                        rp->x = p->getX();
                        rp->y = p->getY();
                        rp->dist = p->getMinDist();
                        result->insert(rp);
                    }
                }
            }

        }
    }

    //    v.setStatistics(visits, count);
    //    cout << "total users = " << count << endl;
    // cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;


    return result;
}

void* Grid::getIDXinRange(double x, double y, double radius, void* posting_list_map){  

    //    Cell* qPoint = NULL;
    //    qPoint=getCell(x,y);
    //    cout<<"query point is in cell: ("<<qPoint->getIndexI()<<" , "<<qPoint->getIndexJ()<<")"<<endl;

    Point* p = NULL;
    Cell* c = NULL;

    unordered_map<int, res_point*>* result = new unordered_map<int, res_point*>();

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    int count = 0;
    int visits = 0;
    int totalcells=0;

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;
	unordered_map<string, unordered_set<int>* >* posting_list_map_cast = (unordered_map<string, unordered_set<int>* >*) posting_list_map;
	
    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            totalcells++;
            c = getCell(i, j);
            count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
				map<string, multiset<tf_pair, pair_descending_frequency>* >* idx = &c->idxOfSet;
				for(auto idx_iter = idx->begin(); idx_iter !=idx->end() ; ++idx_iter){
					string word = idx_iter->first;
					unordered_map<string, unordered_set<int>* >::iterator g_iter = posting_list_map_cast->find(word);
					if(g_iter != posting_list_map_cast->end()){
						unordered_set<int>* users_set = g_iter->second;
						auto l_pl = idx_iter->second;
						for(auto set_iter = l_pl->begin(); set_iter!= l_pl->end();++set_iter){
							tf_pair user = *set_iter;
							users_set->insert(user.getId());
						}
					}
					else{
						unordered_set<int>* users_set = new unordered_set<int>();
						auto l_pl = idx_iter->second;
						for(auto set_iter = l_pl->begin(); set_iter!= l_pl->end();++set_iter){
							tf_pair user = *set_iter;
							users_set->insert(user.getId());
						}
						posting_list_map_cast->insert(make_pair(word,users_set));
					}
				}
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
                visits++;
                list<Point*>* L = c->getCheckIns();
                list<Point*>::iterator it;
                for(it=L->begin(); it != L->end(); ++it){
                    p = *it;
                    //count++;
                    if(p->computeMinDist(x, y) <= radius){
                        res_point* rp = new res_point();
                        rp->id = p->getID();
                        rp->x = p->getX();
                        rp->y = p->getY();
                        rp->dist = p->getMinDist();
                        result->insert(make_pair(rp->id,rp));
                    }
                }
            }

        }
    }

    //    v.setStatistics(visits, count);
    //    cout << "total users = " << count << endl;
    // cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;


    return (void *)result;
}

void* Grid::getIDXinRange_ordered(double x, double y, double radius, void* posting_list_map){  

    //    Cell* qPoint = NULL;
    //    qPoint=getCell(x,y);
    //    cout<<"query point is in cell: ("<<qPoint->getIndexI()<<" , "<<qPoint->getIndexJ()<<")"<<endl;

    Point* p = NULL;
    Cell* c = NULL;

    unordered_map<int, res_point*>* result = new unordered_map<int, res_point*>();

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    int count = 0;
    int visits = 0;
    int totalcells=0;

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;
	unordered_map<string, vector<int>* >* posting_list_map_cast = (unordered_map<string, vector<int>* >*) posting_list_map;
	
    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            totalcells++;
            c = getCell(i, j);
            count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
				map<string, multiset<tf_pair, pair_descending_frequency>* >* idx = &c->idxOfSet;
				for(auto idx_iter = idx->begin(); idx_iter !=idx->end() ; ++idx_iter){
					string word = idx_iter->first;
					unordered_map<string, vector<int>* >::iterator g_iter = posting_list_map_cast->find(word);
					if(g_iter != posting_list_map_cast->end()){
						vector<int>* users_set = g_iter->second;
						auto l_pl = idx_iter->second;
						// users_set->reserve(l_pl->size() + users_set->size());
						for(auto set_iter = l_pl->begin(); set_iter!= l_pl->end();++set_iter){
							tf_pair user = *set_iter;
							users_set->push_back(user.getId());
						}
					}
					else{
						vector<int>* users_set = new vector<int>();
						auto l_pl = idx_iter->second;
						// users_set->reserve(l_pl->size());
						for(auto set_iter = l_pl->begin(); set_iter!= l_pl->end();++set_iter){
							tf_pair user = *set_iter;
							users_set->push_back(user.getId());
						}
						posting_list_map_cast->insert(make_pair(word,users_set));
					}
				}
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
                visits++;
                list<Point*>* L = c->getCheckIns();
                list<Point*>::iterator it;
                for(it=L->begin(); it != L->end(); ++it){
                    p = *it;
                    //count++;
                    if(p->computeMinDist(x, y) <= radius){
                        res_point* rp = new res_point();
                        rp->id = p->getID();
                        rp->x = p->getX();
                        rp->y = p->getY();
                        rp->dist = p->getMinDist();
                        result->insert(make_pair(rp->id,rp));
                    }
                }
            }

        }
    }

    //    v.setStatistics(visits, count);
    //    cout << "total users = " << count << endl;
    // cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;

    return (void *)result;
}


set<res_point*, res_point_ascending_id>* Grid::getSetRange(double x, double y, double radius){

    //    Cell* qPoint = NULL;
    //    qPoint=getCell(x,y);
    //    cout<<"query point is in cell: ("<<qPoint->getIndexI()<<" , "<<qPoint->getIndexJ()<<")"<<endl;

    Point* p = NULL;
    Cell* c = NULL;

    set<res_point*, res_point_ascending_id>* result = new set<res_point*, res_point_ascending_id>();

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    int count = 0;
    int visits = 0;
    int totalcells=0;

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;

    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            totalcells++;
            c = getCell(i, j);
            count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
                visits++;
                list<Point*>* L = c->getCheckIns();
                list<Point*>::iterator it;
                for(it=L->begin(); it != L->end(); ++it){
                    p = *it;
                    //count++;
                    if(p->computeMinDist(x, y) <= radius){
                        res_point* rp = new res_point();
                        rp->id = p->getID();
                        rp->x = p->getX();
                        rp->y = p->getY();
                        rp->dist = p->getMinDist();
                        result->insert(rp);
                    }
                }
            }

        }
    }

    //    v.setStatistics(visits, count);
    //    cout << "total users = " << count << endl;
    //    cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;

    return result;
}



counting_bloom_t* Grid::getRangeBloom(double x, double y, double radius){

    //    Cell* qPoint = NULL;
    //    qPoint=getCell(x,y);
    //    cout<<"query point is in cell: ("<<qPoint->getIndexI()<<" , "<<qPoint->getIndexJ()<<")"<<endl;
	/*
    Cell* c = NULL;

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    if(x_end >= X)
        x_end = X-1;

    if(y_end >= Y)
        y_end = Y-1;

    if(x_start < 0)
        x_start = 0;

    if(y_start < 0)
        y_start = 0;

    int count = 0;
    int totalcells=0;
	
	std::bitset<8> const m("1");
	

    //    cout<<"xstart ="<<x_start<<"  ,  "<<"xend ="<<x_end<<endl;
    //    cout<<"ystart ="<<y_start<<"  ,  "<<"yend ="<<y_end<<endl;
	counting_bloom_t *range_bloom = new_counting_bloom(CAPACITY, ERROR_RATE, "garbage");
	// cout<<"range_bloom->nfuncs "<<range_bloom->nfuncs<<" range_bloom->counts_per_func "<<range_bloom->counts_per_func<<" range_bloom->size "<<range_bloom->size<<" range_bloom->num_bytes "<<range_bloom->num_bytes<<" range_bloom->offset "<<range_bloom->offset<<endl;
	
	// strncpy(range_bloom->bitmap->array, c->getBloomFilter()->bitmap->array, bloomInRange->bitmap->bytes);
	
    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            totalcells++;
            c = getCell(i, j);
            count+=c->getCheckIns()->size();
            if(c != NULL && c->intersectsWithCircle(x, y, radius)){
                //cout<<"cell: "<<"("<<i<<","<<j<<")"<<endl;
				int size = range_bloom->bitmap->bytes;	//say 2
				
				// cout<<"c->getBloomFilter()->nfuncs "<<c->getBloomFilter()->nfuncs<<" c->getBloomFilter()->counts_per_func "<<c->getBloomFilter()->counts_per_func<<" c->getBloomFilter()->size "<<c->getBloomFilter()->size<<" c->getBloomFilter()->num_bytes "<<c->getBloomFilter()->num_bytes<<" c->getBloomFilter()->offset "<<c->getBloomFilter()->offset<<endl;
	
				
				for(int p = 0 ; p < size; ++p){
					bitset<8> const a (range_bloom->bitmap->array[p]);
					bitset<8> const b (c->getBloomFilter()->bitmap->array[p]);
					
					
					std::bitset<8> result;
					for (unsigned int i = 0; i < result.size(); ++i) {
						std::bitset<8> const diff(((a >> i)&m).to_ullong() + ((b >> i)&m).to_ullong() + (result >> i).to_ullong());
						result ^= (diff ^ (result >> i)) << i;
					}
					range_bloom->bitmap->array[p] = static_cast<unsigned char>( result.to_ulong() );
				}
				range_bloom->header->count += c->getBloomFilter()->header->count;
			}
		}
    }
	
	// cout<<"range_bloom->nfuncs "<<range_bloom->nfuncs<<" range_bloom->counts_per_func "<<range_bloom->counts_per_func<<" range_bloom->size "<<range_bloom->size<<" range_bloom->num_bytes "<<range_bloom->num_bytes<<" range_bloom->offset "<<range_bloom->offset<<endl;
	
	
    //    cout << "total users = " << count << endl;
    //    cout << "total cells = " << totalcells << endl;
    //    cout << "result Size = " << result->size() << endl;
    //    cout << "intersected cells = " << visits << endl;

    return range_bloom;
	*/
	//dummy return
	return NULL;
}


// it can be more efficient, check if the circle fully covers the cell
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Here you can avoid computing the minimum distance -----------------------------------------

// panw approach 325 (500) 1151(1000)
// edw me ola 336 (500) 1234(1000)
// xwris to mindist computation twn points 306 (500) 1035(1000)
// me thn nea synerthsh kai to pio panw 310 (500) 1065 (1000)

void Grid::getRange2(double x, double y, double radius, Visitor& v){

    Point* p = NULL;
    Cell* c = NULL;

    int x_start = (int)(((x-radius) - MIN_X)/DELTA_X);
    int x_end = (int)(((x+radius) - MIN_X)/DELTA_X);
    int y_start = (int)(((y-radius) - MIN_Y)/DELTA_Y);
    int y_end = (int)(((y+radius) - MIN_Y)/DELTA_Y);

    for(int i = x_start; i <= x_end; i++){
        for(int j = y_start; j <= y_end; j++){
            c = getCell(i, j);
            if(c != NULL ){
                if(c->fullyCoveredByCircle(x, y, radius)){
                    list<Point*>* L = c->getCheckIns();
                    list<Point*>::iterator it;
                    for(it=L->begin(); it != L->end(); ++it){
                        p = *it;
                        //	p->computeMinDist(x, y);      // ------------------------------------ check times
                        v.setResult(p->getID(), p->getMinDist(), p->getX(), p->getY());
                    }
                }
                else if(c->intersectsWithCircle(x, y, radius)){
                    list<Point*>* L = c->getCheckIns();
                    list<Point*>::iterator it;
                    for(it=L->begin(); it != L->end(); ++it){
                        p = *it;
                        if(p->computeMinDist(x, y) <= radius)
                            v.setResult(p->getID(), p->getMinDist(), p->getX(), p->getY());
                    }
                }
            }
        }

    }
}


bool Grid::loadFromFile(const char* fileName){
    //read file and add to the grid

    ifstream fin(fileName);
    if (! fin)
    {
        std::cerr << "Cannot open data file " << fileName << "." << std::endl;
        return false;
    }

    cout << "Loading Grid ..." << endl;


    int op, id;
    double x1, x2, y1, y2;
    int count = 0;
    Point* u;

    while (fin)
    {
        fin >> op >> id >> x1 >> y1 >> x2 >> y2;
        if (! fin.good()) continue; // skip newlines, etc.
        //	cout << "load id = "<< id << " x = " << x1 << " y " << y1 << endl;
        if(x1> MIN_X && x1 < MAX_X && y1 > MIN_Y && y1 < MAX_Y){
            u = new Point(x1, y1, id);
            addCheckIn(u);
            count ++;
            if(count%100000==0)
                cout << count << endl;
        }
    }

    cout << "Done" << endl;

    return true;
}

/*
TKDE 2004 Tao, papadias
An efficient cost model for optimization of nearest neighbor search in low and medium dimensional
*/
double Grid::estimateNearestDistance(double x, double y, int k){
    //double lx_plus, lx_minus, ly_plus, ly_minus;
    //priority_queue<double, vector<double>, greater<double>> HP;
    int En_old = 0;
    double l_old = 0;
    double Dk, Lr, L;
    int u=0;
    double min;
    double HP[4]; // pairs (position,direction): (0, lx_plus), (1, lx_minus), (2, ly_plus), (3, ly_minus)
    int direction;
    int HP_lastCells[4][2];
    Point* query = new Point(x, y, -1);
    bool validity[4] = {true, true, true, true};
    Cell *queryCell = getCell(x, y);

    for(int i = 0; i < 4; i++){
        HP_lastCells[i][0] = queryCell->getIndexI();
        HP_lastCells[i][1] = queryCell->getIndexJ();
    }

    double Cv = 1.7724538509;            // Cv = sprt(pi) for 2 dimensional scale;

    if(DATASET_SIZE<k)  {
        HP_lastCells[0][0] = X-1;        HP_lastCells[0][1] = Y-1;
        HP_lastCells[1][0] =X-1;        HP_lastCells[1][1] =0;
        HP_lastCells[2][0]= 0;        HP_lastCells[2][1]= 0;
        HP_lastCells[3][0] =0;        HP_lastCells[3][1] =Y-1;
        HP[0] = query->computeMinDist(getCell(HP_lastCells[0][0], HP_lastCells[0][1])->getX2(), getCell(HP_lastCells[0][0], HP_lastCells[0][1])->getY2());
        HP[1] = query->computeMinDist(getCell(HP_lastCells[1][0], HP_lastCells[1][1])->getX2(), getCell(HP_lastCells[1][0], HP_lastCells[1][1])->getY1());
        HP[2] = query->computeMinDist(getCell(HP_lastCells[2][0], HP_lastCells[2][1])->getX1(), getCell(HP_lastCells[2][0], HP_lastCells[2][1])->getY1());
        HP[3] = query->computeMinDist(getCell(HP_lastCells[3][0], HP_lastCells[3][1])->getX1(), getCell(HP_lastCells[3][0], HP_lastCells[3][1])->getY2());

        double max = -1;
        for(int i = 0; i < 4; i++){
            if(max < HP[i]){
                max = HP[i];
            }
        }
        Dk=2*max/Cv;
        /*
        cout <<"--------------------------------------" << endl;
        cout<<"Data Dependent Estimation Completed with"<<endl;
        cout<<"Estimated radius (Dk) of circle = " << Dk <<endl;
        cout<< "Actual Users in the estimated radius = " << getRange(x, y, Dk)->size() << endl;
        cout<< "--------------------------------------" << endl;
        */
        return Dk;
    }

    do{

        HP[0] = query->computeMinDist(getCell(HP_lastCells[0][0], HP_lastCells[0][1])->getX2(), y);
        HP[1] = query->computeMinDist(getCell(HP_lastCells[1][0], HP_lastCells[1][1])->getX1(), y);
        HP[2] = query->computeMinDist(x, getCell(HP_lastCells[2][0], HP_lastCells[2][1])->getY2());
        HP[3] = query->computeMinDist(x, getCell(HP_lastCells[3][0], HP_lastCells[3][1])->getY1());

        min = MAXDIST;
        for(int i = 0; i < 4; i++){

            //            cout << "Min("<< i << ") = " << HP[i] << endl;

            if(min > HP[i] && validity[i]){
                direction = i;
                min = HP[i];
            }
        }
        L = 2*min;

        list<Cell*>* intersectedCells = getIntersectedCellsWithRectangle(x-L/2+BOUNDARY_ERROR, y-L/2+BOUNDARY_ERROR, x+L/2-BOUNDARY_ERROR, y+L/2-BOUNDARY_ERROR);

        int usersInIntersectedCells = 0;
        u=0;
        double intersectedVolPercentage = 0;

        for(list<Cell*>::iterator it = intersectedCells->begin(); it!= intersectedCells->end(); ++it){
            Cell *tmp = *it;

            intersectedVolPercentage = tmp->intersectedVolumeWithRectangle(x-L/2, y-L/2, x+L/2, y+L/2)/(DELTA_X*DELTA_Y);
            if(intersectedVolPercentage > 1)
                intersectedVolPercentage = 1;

            u+= ceil(tmp->getCheckIns()->size()*intersectedVolPercentage);
            usersInIntersectedCells+=tmp->getCheckIns()->size();
        }

        //u=(int)usersCount;

        if(k <= u){
            // compute Lr
            Lr = pow((l_old*l_old*(k-u)-L*L*(k-En_old))/(En_old-u), 0.5);
            // cout << "Total # of intersected cells = " << intersectedCells->size()<<endl;
            //cout<< "--------------------------------------" << endl;
            //cout<< "Total number of users in the intersected cells = " << usersInIntersectedCells << endl;

            // compute Dk
            Dk = Lr/Cv;
            /*
            cout <<"--------------------------------------" << endl;
            cout<<"Data Dependent Estimation Completed with"<<endl;
            cout<<"Estimated radius (Dk) of circle = " << Dk <<endl;
            cout<< "--------------------------------------" << endl;
            cout<<"Estimated number of users enclosed(En) in the rectangle"<<endl;
            cout<<"with extent Lr = "<<Lr<<" are within " << En_old <<" and "<<u<<"."<< endl;
            cout<< "--------------------------------------" << endl;
            cout<< "Actual Users in the estimated radius = " << getRange(x, y, Dk)->size() << endl;
            cout<< "--------------------------------------" << endl;
                         */
            return Dk;
        }
        else{
            l_old = L;
            En_old = u;
            bool flag = false;

            if(direction == 0){
                if(HP_lastCells[0][0] > 0 && HP_lastCells[0][0] < X-1){
                    HP_lastCells[0][0] = HP_lastCells[0][0] +1;
                    flag =true;
                }
            }
            else if(direction == 1){
                if(HP_lastCells[1][0] > 0 && HP_lastCells[1][0] < X-1){
                    HP_lastCells[1][0] = HP_lastCells[1][0] -1;
                    flag =true;
                }
            }
            else if(direction == 2){
                if(HP_lastCells[2][1] > 0 && HP_lastCells[2][1] < Y-1){
                    HP_lastCells[2][1] = HP_lastCells[2][1] +1;
                    flag =true;
                }
            }
            else {
                if(HP_lastCells[3][1] > 0 && HP_lastCells[3][1] < Y-1){
                    HP_lastCells[3][1] = HP_lastCells[3][1] -1;
                    flag =true;
                }
            }
            if(!flag)
                validity[direction] = false;
        }
    }while(true);
}