#include "../../../headersMemory.h"

Cell::Cell(){
    cell_type = CELL;
    checkIns = new list<Point*>();
	idxImpact = new unordered_map<string,double>();
	idxOfImpacts = new unordered_map<string , multiset<my_pair, pair_comparator_descending>* >();
	
	points_of_interest = new list<Point*>();
	P_idxImpact = new unordered_map<string,double>();
	P_idxOfImpacts = new unordered_map<string , multiset<my_pair, pair_comparator_descending>* >();
    // ---------------------------------
    x1 = new double;
    x2 = new double;                  //  |-----X2
    y1 = new double;                  //  |     |
    y2 = new double;                  //  X1----|
	
	history_checkin_count = 0;
    numOfPoints = 0;
	max_degree = 0;
	filter = NULL;
	hasFilter = false;
}

Cell::~Cell(){

    delete x1;
    delete x2;
    delete y1;
    delete y2;


    while(!checkIns->empty()) {
        delete checkIns->back();
        checkIns->pop_back();
    }
    delete checkIns;
	
	//delete bloom filter
	// if(cell_type == CELL)
		// free_counting_bloom(bloom);
		
	//delete scaling bloom
	// if(cell_type == CELL)
		// free_scaling_bloom(bloom);
		
		
	if(cell_type == CELL)
		delete filter;
};


void Cell::deleteCheckIn(Point* p){
    //cout << "Delete checkin" << endl;
    for(list<Point*>::iterator it = checkIns->begin(); it != checkIns->end(); ++it){
        if((*it)->getID() == p->getID()){
            //cout << "To vrika!" << endl;
            checkIns->erase(it);
            break;
        }
    }
}


void Cell::setIndex(int c_i, int c_j){
    i = c_i;
    j = c_j;
}


int Cell::getIndexI(){
    return i;
}

int Cell::getIndexJ(){
    return j;
}

scaling_bloom_t * Cell::getBloomFilter(){
	return bloom;
}

void Cell::setDimensions(double x_1, double y_1, double x_2, double y_2){
    *x1 = x_1;
    *x2 = x_2;
    *y1 = y_1;
    *y2 = y_2;

    /*	x1 = x_1;
        x2 = x_2;
        y1 = y_1;
        y2 = y_2;
*/
}

bool Cell::isUserInCell(int f_id){
	// cout<<"users in bloom of cell: <"<<i<<","<<j<<"> = "<<bloom->header->count<<" | #checkins = "<<checkIns->size()<<endl;

	ostringstream ss;
	ss << f_id;
	string f_id_s = ss.str();
	return scaling_bloom_check(bloom, f_id_s.c_str(), strlen(f_id_s.c_str()));
}

void Cell::newCheckIn(Point* user, int degree){
	//add user to bloom
	// ostringstream ss;
    // ss << user->getID();
	// string user_id = ss.str();
	
	
	if(degree > max_degree){
		max_degree = degree;
	}
	// cout<<"New checkin in cell: <"<<i<<","<<j<<"> = "<<user_id<<" max_degree changed to "<<max_degree<<endl;

	// counting_bloom_add(bloom, user_id.c_str(), strlen(user_id.c_str()));
	
	//add to grid
    checkIns->push_back(user);
} 

void Cell::newSIDX(string word, int occurrences, int user_id){
	
	map<string, multiset<tf_pair, pair_descending_frequency>* >::iterator it;
	it = idxOfSet.find(word);
	if(it != idxOfSet.end()) {                // word seen
        multiset<tf_pair, pair_descending_frequency>* posting_list_SET = (*it).second;
		posting_list_SET->insert(tf_pair(user_id, occurrences) );
    }
    else {                                           // word NOT seen
		multiset<tf_pair, pair_descending_frequency>* posting_list_SET = new multiset<tf_pair, pair_descending_frequency>();
		posting_list_SET->insert(tf_pair(user_id, occurrences) );
		idxOfSet.insert (make_pair(word, posting_list_SET));
	}
	
	//manage document length of all users involved.
	// need term frequency only
	auto iter = docLenMap.find(user_id);
	double tf_w = 1+log10(occurrences);
	
	if(iter==docLenMap.end()){
		// if(user_id == 175){
			// cout<<" adding to map for user: "<<user_id<<" doc_len: "<< tf_w*tf_w <<" word: "<<word <<" tf_w: " <<tf_w<<endl;
		// }
		docLenMap.insert(make_pair(user_id, tf_w*tf_w));
	}
	else{
		// if(user_id == 175){
			// cout<<" incrementing in map for user: "<<user_id<<" from doc_len: "<< iter->second<< " to "<<iter->second + (tf_w*tf_w)<<" word: "<<word <<" tf_w: " <<tf_w<<endl;
		// }
		iter->second = iter->second + (tf_w*tf_w);
	}
}

void Cell::newPOI(Point* poi){
	//add user to bloom
	// ostringstream ss;
    // ss << poi->getID();
	// string poi_id = ss.str();
	// cout<<"New checkin in cell: <"<<i<<","<<j<<"> = "<<user_id<<" max_degree changed to "<<max_degree<<endl;
	// counting_bloom_add(bloom, user_id.c_str(), strlen(user_id.c_str()));
	// cout<<"\t\t adding POI to cell: "<<poi->getID()<<endl;
	points_of_interest->push_back(poi);
} 

void Cell::newPOIHistoryEntry(int poi_id, int history_checkin){
	
	
	if(filter == NULL){
		// set bloom filter parameters
	    bloom_parameters parameters;
	     // How many elements roughly do we expect to insert?
	    parameters.projected_element_count = 50000;
	    // Maximum tolerable false positive probability? (0,1)
	    parameters.false_positive_probability = 0.1; // 100 in 10000
	    // Simple randomizer (optional)
	    parameters.random_seed = 0xA5A5A5A5;

	    if (!parameters)
	    {
		   std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
		   return;
	    }
	    parameters.compute_optimal_parameters();
	    // Instantiate Bloom Filter
	    filter = new bloom_filter(parameters);
	}
	
	//add history_checkin to unordered_map of POI, to enable set intersections

	auto it = P_historyMap.find(poi_id);
	if(it != P_historyMap.end()){
		unordered_set<int>* history_set = it->second;
		history_set->insert(history_checkin);
	}
	else{
		unordered_set<int>* history_set = new unordered_set<int>();
		history_set->insert(history_checkin);
		P_historyMap.insert(make_pair(poi_id,history_set));
	}
	
	//add history record to bloom filter of the all the cells in the height of the tree

	//FOR SCALING BLOOM FILTER
	// ostringstream ss;
    // ss << poi_id;
	// const std::string& tmp = ss.str();
	// const char* poi_id_ = tmp.c_str();
	// history_checkin_count++;
	// scaling_bloom_add(this->bloom, poi_id_, strlen(poi_id_),history_checkin_count);
	
	// QuadTreeNode* parent = pParentNode;
	// while(parent!= NULL){
		// int hc = parent->history_checkin_count;
		// scaling_bloom_add(parent->bloom, poi_id_, strlen(poi_id_),parent->history_checkin_count);
		// parent->history_checkin_count = hc + 1;
		// parent = parent->pParentNode;
	// }

	// cout << "adding poi: "<<poi_id<<" history checkin: "<<history_checkin<<"....filter size is now: "<<filter->element_count()<<endl;
	//FOR TRUE BLOOM FILTER
	this->filter->insert(history_checkin);
	// if(!(this->filter->contains(history_checkin))){
		// cout<<"\t FALSE NEGATIVE"<<endl;
	// }
	QuadTreeNode* parent = pParentNode;
	while(parent!= NULL){
		parent->filter->insert(history_checkin);
		// if(!(parent->filter->contains(history_checkin))){
			// cout<<"\t FALSE NEGATIVE"<<endl;
		// }
		parent = parent->pParentNode;
	}

}

void Cell::newPOIidxEntry(string word, int occurrences, int poi_id){
	
	map<string, multiset<tf_pair, pair_descending_frequency>* >::iterator it;
	 multiset<tf_pair, pair_descending_frequency>* posting_list_SET;
	it = P_idxOfSet.find(word);
	if(it != P_idxOfSet.end()) {                // word seen
        posting_list_SET = (*it).second;
		posting_list_SET->insert(tf_pair(poi_id, occurrences) );
    }
    else {                                           // word NOT seen
		posting_list_SET = new multiset<tf_pair, pair_descending_frequency>();
		posting_list_SET->insert(tf_pair(poi_id, occurrences) );
		P_idxOfSet.insert (make_pair(word, posting_list_SET));
	}
	
	//manage document length of all users involved.
	// need term frequency only
	auto iter = P_docLenMap.find(poi_id);
	double tf_w = 1+log10(occurrences);
	
	if(iter==P_docLenMap.end()){
		P_docLenMap.insert(make_pair(poi_id, tf_w*tf_w));
	}
	else{
		iter->second = iter->second + (tf_w*tf_w);
	}
	
	// cout<<"adding to cell: <"<<i<<","<<j<<"> word for POI: "<<poi_id<<" -> "<<word<<" -freq- "<<occurrences<<" P_idxOfSet.size(): "<<P_idxOfSet.size()<<" posting list size: "<<posting_list_SET->size()<<endl;
}

double Cell::getDocLen(int _user_id){
	auto it = docLenMap.find(_user_id);
	if(it!= docLenMap.end())
		return it->second;
	else	
		return 999999;
}

list<Point*>* Cell::getCheckIns(){
    return checkIns;
}

list<Point*>* Cell::getPointsOfInterest(){
	return points_of_interest;
}

multiset<tf_pair, pair_descending_frequency>* Cell::lookupWord(const string& word) {
	// cout<<"size of idx in cell: <"<<i<<","<<j<<"> = "<<idxOfSet.size()<<endl;
	
	auto it = idxOfSet.find(word);
	if(it != idxOfSet.end())
		return it->second;
	else
		return NULL;
}

multiset<my_pair, pair_comparator_descending>* Cell::lookupWordInImpacts(const string& word) {
	// cout<<"size of idx in cell: <"<<i<<","<<j<<"> = "<<idxOfImpacts.size()<<endl;
	auto it = idxOfImpacts->find(word);
	if(it != idxOfImpacts->end())
		return it->second;
	else
		return NULL;
}

multiset<my_pair, pair_comparator_descending>* Cell::lookupWordIn_P_Impacts(const string& word) {
	// cout<<"size of idx in cell: <"<<i<<","<<j<<"> = "<<idxOfImpacts.size()<<endl;
	auto it = P_idxOfImpacts->find(word);
	if(it != P_idxOfImpacts->end())
		return it->second;
	else
		return NULL;
}


void Cell::setType(int type){
	// stringstream ss;
	// ss << "garbage/" << i << "_" << j;
	// string filename = ss.str();
	// const char * c = filename.c_str();
	// if(type == CELL){
		// initialize bloom filter
		// if (!(bloom = new_counting_bloom(CAPACITY, ERROR_RATE, "bloomfile"))) {
			// fprintf(stderr, "ERROR: Could not create bloom filter for cell <"<< i << "_" << j<<">\n");
		// }
	// }
	/*
	if(type == CELL){
		// set bloom filter parameters
	    bloom_parameters parameters;
	     // How many elements roughly do we expect to insert?
	    parameters.projected_element_count = 1000;
	    // Maximum tolerable false positive probability? (0,1)
	    parameters.false_positive_probability = 0.1; // 100 in 10000
	    // Simple randomizer (optional)
	    parameters.random_seed = 0xA5A5A5A5;

	    if (!parameters)
	    {
		   std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
		   return;
	    }
	    parameters.compute_optimal_parameters();
	    // Instantiate Bloom Filter
	    filter = new bloom_filter(parameters);
	}
	*/
	
    cell_type = type;
}

int Cell::getType(){
    return cell_type;
}


//----------------------------------
double Cell::getX1(){ return *x1;}

double Cell::getY1(){ return *y1;} 

double Cell::getX2(){ return *x2;}

double Cell::getY2(){ return *y2;}          

void Cell::setID(int id){
    c_id = id;
}

int Cell::getID(){
    return c_id;
}


//---------------------------------

double Cell::computeMinDist(double x, double y){

    c_minDist = 0.0;

    if (x < *x1){
        c_minDist += std::pow(*x1 - x, 2.0);
    }
    else if (x > *x2){
        c_minDist += std::pow(x - *x2, 2.0);
    }

    if (y < *y1){
        c_minDist += std::pow(*y1 - y, 2.0);
    }
    else if (y > *y2){
        c_minDist += std::pow(y - *y2, 2.0);
    }

    c_minDist = sqrt(c_minDist);

    return c_minDist;
}

double Cell::computeMaxDist(double x, double y){

	//  D(x1,y2)----C(x2,y2)
	//  |	        |
	//  A(x1,y1)----B(x2,y1)
	
	//comment denote position of point NOT of cell

    double dx, dy;
    if (x < *x1) { // left hemisphere
        dx = *x2 - x;
        if (y < *y1) { // lower left quadrant
            dy = *y2 - y; //distance to point C
        }
        else if (y > *y2) { // upper left quadrant
            dy = y - *y1; // distance to point B
        }
        else { // immediate left - can be either point B or C
			dy = max(y - *y1,*y2 - y);
        }
    }
    else if (x > *x2) { // right hemisphere
        dx = x - *x1;
        if (y < *y1) { // lower right quadrant
            dy = *y2 - y; // distance to point D
        }
        else if (y > *y2) { // upper right quadrant
            dy = y - *y1; // distance to point A
        }
        else {
			dy = max(y - *y1,*y2 - y); // immediate right - can be either point A or D.
        }
    }
    else {
        if (y < *y1) { //immediately below - either point C or D
			dx = max(*x2 - x,x - *x1);
			dy = *y2 - y;
        }
        else if (y > *y2) { //immediately above - either point A or B
			dx = max(*x2 - x,x - *x1);
			dy = y - *y1;
        }
        else {
			dx = max(*x2 - x,x - *x1);
			dy = max(*y2 - y,y - *y1); // inside the rectangle or on the edge - can be any corner
        }
    }

	c_maxDist = sqrt(dx * dx + dy * dy);
    return c_maxDist;
}   




bool Cell::intersectsWithCircle(double x, double y, double radius){

    double dist = computeMinDist(x, y);

    if (dist - radius > 0)
        return false;

    return true;
}


double Cell::intersectedVolumeWithRectangle(double x_1, double y_1, double x_2, double y_2){

    if(x_1 < MIN_X)
        x_1 = MIN_X;
    else if(x_1 > MAX_X)
        x_1 = MAX_X;

    if(x_2 < MIN_X)
        x_2 = MIN_X;
    else if(x_2 > MAX_X)
        x_2 = MAX_X;

    if(y_1 < MIN_Y)
        y_1 = MIN_Y;
    else if(y_1 > MAX_Y)
        y_1 = MAX_Y;

    if(y_2 < MIN_Y)
        y_2 = MIN_Y;
    else if(y_2 > MAX_Y)
        y_2 = MAX_Y;


    double x_overlap = max(0.0, abs(min(*x2, x_2) - max(*x1, x_1)));
    double y_overlap = max(0.0, abs(min(*y2, y_2) - max(*y1, y_1)));

    /*
    if(x_overlap - DELTA_X > 0)
        cout << " ---------------------------Error x_overlap = " << x_overlap << "\t DELTA_X = " << DELTA_X << endl;
    if(y_overlap - DELTA_Y > 0)
        cout << " ---------------------------Error y_overlap = " << y_overlap << "\t DELTA_Y = " << DELTA_Y << endl;

    */
    return x_overlap*y_overlap;
}

//----------------------------------------------

// Check if all cells corners lay into the circle
bool Cell::fullyCoveredByCircle(double x, double y, double radius){

    if(sqrt(pow(x-*x1, 2.0) + pow(y-*y1, 2.0)) <= radius){
        if(sqrt(pow(x-*x1, 2.0) + pow(y-*y2, 2.0)) <= radius){
            if(sqrt(pow(x-*x2, 2.0) + pow(y-*y1, 2.0)) <= radius){
                if(sqrt(pow(x-*x2, 2.0) + pow(y-*y2, 2.0)) <= radius){
                    return true;
                }else
                    return false;
            }else
                return false;
        }else
            return false;
    }else
        return false;


}


void Cell::setDirection(int direction){
    c_direction = direction;
} 

int Cell::getDirection(){
    return c_direction;
}     

double Cell::getMinDist(){
    return c_minDist;
}  

void Cell::setMinDist(double dist){
    c_minDist = dist;
}          
