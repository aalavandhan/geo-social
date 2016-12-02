#include "../../headersMemory.h"

GPOs::GPOs (char* gridFileName, const string& keywordfile, SPOs* _spos){

    //pages = (int*) malloc(sizeof(int));
    //pages[0] = 0;

    kNNExecutions = 0;
    LocationExecutions = 0;
    NextNNExecutions = 0;
    RangeExecutions = 0;
	pureNNexec = 0;

    totalCPUTime = totalTime = 0.0;
	
    grid = new Grid();
    //  grid->loadFromFile(gridFileName);
	sendSPOStoGRID(_spos);
	spos = _spos;
    //locations = new map<int, Point*>(); ----
    loadLocations(gridFileName);
	// loadIDF(IDFfileName);
	loadTextFile(keywordfile);
	
    objects = 0;
    // nextNN without incremental approach of NN
    computedNN = returnedNN = finalNextNN = 0;
    nextNNList = new vector<res_point*>();
    flagNextNN = true;
	// rootNode = grid->createQuadTree(5);
}

//load POIs, their keywords, idf and fan set
void GPOs::loadPOIeverything(const char* locationFile, const char* historyFile, const char* keywordfile, const char* frequencyFile){
	
	//locations of POIs
	{
		ifstream fin(locationFile);
		if (! fin)
		{
			std::cerr << "Cannot open  file " << locationFile << std::endl;
		}

		cout << "HashTable: [ POI_ID -> location ] Loading from " << locationFile << endl;

		int op, id;
		double x1, x2, y1, y2;
		Point* poi;

		while (fin){
		
			fin >> op >> id >> x1 >> y1 >> x2 >> y2;
			if (! fin.good()) continue; // skip newlines, etc.
			poi = new Point(x1, y1, id);

			//locations2[id] = u;
			poi_locations.insert(pair<int, Point*>(id, poi));
			grid->addPOI(poi);
			// ids.push_back(id);

		}

		fin.close();
		cout << "Done! Num of POIs in map: " <<  poi_locations.size() << endl;
	}
	
	//keywords of POIs
	{
		cout << "Loading P_keywords from file " << keywordfile << endl;
		ifstream infile(keywordfile);
		int count = 0;
		if (infile) {
			string line;
			while (getline(infile, line)) {
				char *bigBuffer = new char[line.size()+1];
				bigBuffer[line.size()]=0;
				memcpy(bigBuffer, line.c_str(),line.size());
				char *split;
				char *saveptr;
				int i =0;
				split = strtok_r(bigBuffer, "|",&saveptr);
				string word;
				int poi_id;
				unordered_set<string>* profile_keyword_list = new unordered_set<string>();
				while(split != NULL){
					//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
					if(strlen(split) > 0){
						if(i==0){
							poi_id = atoi(split);
							
						}
						else if(i % 2 == 1){
							string temp(split);
							word = temp;
							profile_keyword_list->insert(word);		
						}
						else{
							int occurrences = atoi(split);
							auto it = poi_locations.find(poi_id);
							if(it!=poi_locations.end()){
								// cout<<"adding to grid word for POI: "<<poi_id<<" -> "<<word<<" Freq: "<<occurrences<<endl;
								grid->addPOIToSIDX(word, occurrences, (*it).second);

								auto pit = P_IDF.find(word);
								if(pit!=P_IDF.end()){
									pit->second = pit->second + 1;
								}else{								
									P_IDF.insert(make_pair(word,1));
								}
							}
							else{
								cout<<"Cannot insert into SIDX. poi not in grid."<<endl;
							}
						}
					}
					split = strtok_r(NULL, "|",&saveptr);
					i++;
				}
				poi_profile_map->insert(make_pair(poi_id, profile_keyword_list));	
				count++;
			}

			cout<< "Parsed lines/users in keyword file: "<<count<<endl;
		}
		else {
			cerr << "can't open file " << keywordfile << endl;
		}
	}
	
	
	//IDF of POIs
	// {
		// cout << "Loading P_IDF from file " << frequencyFile << endl;
		// ifstream infile(frequencyFile);
		// if (infile) {
			// string line;
			// while (getline(infile, line)) {
				// char *bigBuffer = new char[line.size()+1];
				// bigBuffer[line.size()]=0;
				// memcpy(bigBuffer, line.c_str(),line.size());
				// char *split;
				// char *saveptr;
				// int i =0;
				// split = strtok_r(bigBuffer, "|",&saveptr);
				// string word;
				// int document_frequency;
				
				// while(split != NULL){
							   // //cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
					// if(strlen(split) > 0){
						// if(i==0){
							// string temp(split);
							// word = temp;
						// }
						// else if(i % 2 == 1){
							// document_frequency = atoi(split);
							// P_IDF.insert(pair<string, int>(word, document_frequency) );
						// }
					// }
					// split = strtok_r(NULL, "|",&saveptr);
					// i++;
				// }
			// }

			// cout<< "Done...parsed terms in P_IDF file: "<<P_IDF.size()<<endl;
		// }
		// else {
			// cerr << "can't open file " << frequencyFile << endl;
		// }
	// }
	
	cout<<"Done...parsed terms in P_IDF file: "<<P_IDF.size()<<endl;
	
	//create quad tree
	loadHistoryCheckins(historyFile);
}

void GPOs::createQuadTree(){
	rootNode = grid->createQuadTree();
}

void GPOs::loadHistoryCheckins(const char * historyFile){
	//history checkins of POIs load like social graph 
	//in the POI query we will do set intersection between N(v) and V_p
	//quickest solution is to create hash map at cell and bloom filters in the quad tree
	
	ifstream fin(historyFile);
	if (!fin){
		cout << "Cannot open file " << historyFile << endl;
	}
	
	int history_checkin_accumulator = 0;
	int poi_id, size;
	int pois=0;
	while(fin){ //NUMOFUSERS
		fin >> poi_id >> size;
		if (! fin.good())continue;	// skip newlines, etc.
		
		unordered_set<int>* poi_history_set = NULL;
		if(size>0)
			poi_history_set = new unordered_set<int>();
			
		for(int i = 0; i < size; i++){
			int history_checkin;
			fin >> history_checkin;
			grid->newPOIHistoryEntry(poi_id,history_checkin);
			poi_history_set->insert(history_checkin);
			++history_checkin_accumulator;
		}
		POI_historyMap.insert(make_pair(poi_id,poi_history_set));
		
		// history_checkin_accumulator += history_set->size();
		// grid->setHistoryPOI(poi_id,history_set);
		pois++;
	}
	fin.clear();
	fin.close();

	cout << "POIs loaded: "<<pois<<" with total History checkins: "<<history_checkin_accumulator<<endl;
}


unordered_set<int>* GPOs::getHistoryCheckins(int poi_id){
	auto it = POI_historyMap.find(poi_id);
	if(it != POI_historyMap.end()){
		return it->second;
	}
	return NULL;
}

unordered_map<int, Point*>* GPOs::getAllPOIlocations(){
	return &poi_locations;
}


Cell* GPOs::getCell(double x, double y){
	return grid->getCell(x,y);
}

void GPOs::sendSPOStoGRID(SPOs* _spos) {
	grid->setSPOs(_spos);
}

vector <QuadTreeNode*>* GPOs::getQuadTree(){
	return rootNode;
}

bool GPOs::loadTextFile(const string& keywordfile) {
	cout << "Loading keywords in GPOs from file " << keywordfile << endl;
	ifstream infile(keywordfile.c_str());
	int count = 0;
	if (infile) {
		string line;
		while (getline(infile, line)) {
			char *bigBuffer = new char[line.size()+1];
			bigBuffer[line.size()]=0;
			memcpy(bigBuffer, line.c_str(),line.size());
			
			char *split;
			char *saveptr;
			int i =0;
			split = strtok_r(bigBuffer, "|",&saveptr);
			string word;
			int user_id;
			
			unordered_set<string>* profile_keyword_list = new unordered_set<string>();

			while(split != NULL){
				//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
				if(strlen(split) > 0){
					if(i==0){
						user_id = atoi(split);
					}
					else if(i % 2 == 1){
						string temp(split);
						word = temp;
						profile_keyword_list->insert(word);
					}
					else{
						int occurrences = atoi(split);
						auto it = locations.find(user_id);
						if(it!=locations.end()){
							grid->addToSIDX(word, occurrences, (*it).second);
						}
						else{
							cout<<"Cannot insert into SIDX. User not in grid."<<endl;
						}
						auto pit = IDF.find(word);
						if(pit!=IDF.end()){
							pit->second = pit->second + 1;
						}else{								
							IDF.insert(make_pair(word,1));
						}
					}
				}
				split = strtok_r(NULL, "|",&saveptr);
				i++;
			}
			//create user profile
			user_profile_map->insert(make_pair(user_id, profile_keyword_list));			
			count++;
		}

		cout<< "Parsed lines/users in keyword file: "<<count<<endl;
		return true;
	}
	else {
		cerr << "can't open file " << keywordfile << endl;
		return false;
	}
}

bool GPOs::loadIDF(const char* fileName){

	cout << "Loading IDF in GPOs from file " << fileName << endl;
	ifstream infile(fileName);
	int count = 0;
	if (infile) {
		string line;
		while (getline(infile, line)) {
			char *bigBuffer = new char[line.size()+1];
			bigBuffer[line.size()]=0;
			memcpy(bigBuffer, line.c_str(),line.size());
			char *split;
			char *saveptr;
			int i =0;
			split = strtok_r(bigBuffer, "|",&saveptr);
			string word;
			int document_frequency;
			
			while(split != NULL){
				//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
				if(strlen(split) > 0){
					if(i==0){
						string temp(split);
						word = temp;
					}
					else if(i % 2 == 1){
						document_frequency = atoi(split);
						IDF.insert(pair<string, int>(word, document_frequency) );
					}
				}
				split = strtok_r(NULL, "|",&saveptr);
				i++;
			}
			count++;
		}

		cout<< "Parsed terms in IDF file: "<<count<<endl;
		return true;
	}
	else {
		cerr << "can't open file " << fileName << endl;
		return false;
	}
	
	return true;
}

int GPOs::getP_IDF(string word){
	auto it = P_IDF.find(word);
	if(it != P_IDF.end()){
		return it->second;
	}else{
		cout<<"query word not in vocabulary"<<endl;
		return 0;
	}
}

int GPOs::getIDF(string word){
	auto it = IDF.find(word);
	if(it != IDF.end()){
		return it->second;
	}else{
		cout<<"query word not in vocabulary"<<endl;
		return 0;
	}
}


unordered_set<string>* GPOs::getUserProfile(int _id){
	unordered_set<string>* user_profile = NULL;
	auto it = user_profile_map->find(_id);
	if(it != user_profile_map->end()){
		user_profile = it->second; 
	}
	return user_profile;
}

unordered_set<string>* GPOs::getPOIProfile(int _id){
	unordered_set<string>* poi_profile = NULL;
	auto it = poi_profile_map->find(_id);
	if(it != poi_profile_map->end()){
		poi_profile = it->second; 
	}
	return poi_profile;
}

GPOs::GPOs (){

    //pages = (int*) malloc(sizeof(int));
    //pages[0] = 0;

    kNNExecutions = 0;
    LocationExecutions = 0;
    NextNNExecutions = 0;
    RangeExecutions = 0;
	pureNNexec = 0;

    totalCPUTime = totalTime = 0.0;

    grid = new Grid;

    //locations = new map<int, Point*>(); ----
    objects = 0;

    // nextNN without incremental approach of NN
    computedNN = returnedNN = finalNextNN = 0;
    nextNNList = new vector<res_point*>();
    flagNextNN = true;
}


GPOs::~GPOs (){

    delete grid;
    delete &locations;
}


double GPOs::getTotalCPUTime(){
    return totalCPUTime;
}

double GPOs::getTotalTime(){
    return totalTime;
}


void GPOs::getLocation(int id, double* result){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();
    LocationExecutions++;

    auto it = locations.find(id);

    if(it!=locations.end()){
        result[0]= (*it).second->getX();
        result[1]= (*it).second->getY();
    }
    else{
        result[0] = -1000;
    }

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

}


counting_bloom_t* GPOs::getRangeBloom(double x, double y, double radius){
	return grid->getRangeBloom(x,y,radius);
}

bool GPOs::isUserInRange(double x, double y, double radius, int f_id){
	return grid->isUserInRange(x,  y,  radius,  f_id);
}

res_point* GPOs::getNextNN(double x, double y){

	if(pureNNexec == 0)
		incrVis = new IncrVisitor(x, y);
		
	grid->getNextNN(*incrVis, 1);	

	pureNNexec++;

	return incrVis->getNext();
}

vector<res_point*>* GPOs::getkNNfromIDX(double x, double y, int k, vector<string>* terms){
	return grid->getkNNfromIDX( x, y,  k,  terms);
}

res_point* GPOs::getNextNearestUser(double x, double y, int signal){
	return grid->getNextNearestUser(x,y,signal);
}


res_point* GPOs::getNextNearestNeighbourWithTerm(double x, double y, int signal, vector<string>* terms){
	return grid->getNextNearestNeighbourWithTerm(x, y, signal, terms);
}


res_point* GPOs::getNextNN(double x, double y, int incrStep){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();


    if(computedNN <= returnedNN && flagNextNN){
        NextNNExecutions++;
        computedNN+=incrStep;
//        cout<<"returneNN="<<returnedNN<<endl;
//        cout << "next-" << computedNN << endl;
        vector<res_point*>* kNN = grid->getkNN(x, y, computedNN);
        int size = kNN->size();
//        cout << "size = " << size << endl;

        for(int i = returnedNN; i < size; i++){
            nextNNList->push_back(util.copy((*kNN)[i]));
        }

        while(!kNN->empty()) {
            delete kNN->back();
            kNN->pop_back();
        }
        delete kNN;

        int newNNsize = nextNNList->size();
//        cout << "newNNsize = " << newNNsize << endl;
//        cout << "computted NN = "<<computedNN<<endl;
        if(computedNN > newNNsize){ // no more!
            //cout<<"here"<<endl;
            flagNextNN = false;
            computedNN = newNNsize;
        }
    }


    if(computedNN > returnedNN){
        endC = clock();
        totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
        return (*nextNNList)[returnedNN++];
    }
    else{
        endC = clock();
        totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
        return NULL;
    }
}


vector<res_point*>* GPOs::getkNN(double x, double y, int k){

    //	cout << "getKNN start" << endl;
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    kNNExecutions++;

//        cout << "----" << k << endl;
    vector<res_point*>* res = grid->getkNN(x, y, k);
//        cout << "size = " << res->size() << endl;


    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);
    //	cout << "getKNN end" << endl;

    return res;
}


vector<res_point*>* GPOs::getRange(double x, double y, double radius){
    struct timeval start, end;
    gettimeofday(&start, NULL);

    RangeExecutions++;

    vector<res_point*>* res = grid->getRange(x, y, radius);

    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

    //    cout << "Num of users (GPOs) in radius = "<<radius<<" are =" << res->size() << endl;
    return res;
}


set<res_point*, res_point_ascending_dist>* GPOs::getRangeWithIDX(double x, double y, double radius, vector<string> *terms, vector<vector<multiset<tf_pair, pair_descending_frequency>*>*>* _keywordVEC){
    return grid->getRangeWithIDX(x, y, radius,terms, _keywordVEC);
}

void* GPOs::getIDXinRange(double x, double y, double radius, void* posting_list_map){
	return grid->getIDXinRange(x, y, radius, posting_list_map);
}
	

void* GPOs::getIDXinRange_ordered(double x, double y, double radius, void* posting_list_map){
	return grid->getIDXinRange_ordered(x, y, radius, posting_list_map);
}
	

set<res_point*, res_point_ascending_id>* GPOs::getSetRange(double x, double y, double radius){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    RangeExecutions++;

    set<res_point*, res_point_ascending_id>* res = grid->getSetRange(x, y, radius);

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

    //    cout << "Num of users (GPOs) in radius = "<<radius<<" are =" << res->size() << endl;
    return res;
}




vector<res_point*>* GPOs::getRangeSortedId(double x, double y, double radius){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    RangeExecutions++;

    vector<res_point*>* res = grid->getRange(x, y, radius);
    util.sortResPoint_AscendingId(res);

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);
    return res;
}

/*
int GPOs::getPageAccesses(){
    return pages[0];
}

void GPOs::resetPageAccesses(){
    pages[0] = 0;
}
*/

int GPOs::getkNNExecutions(){
    return kNNExecutions;
}

int GPOs::getLocationExecutions(){
    return LocationExecutions;
}

int GPOs::getNextNNExecutions(){
    return NextNNExecutions;
}

int GPOs::getRangeExecutions(){
    return RangeExecutions;
}

void GPOs::updateCheckin(Point* p){
    //grid->updateCheckIn(p, locations2[p->getID()]->getX(), locations2[p->getID()]->getY());
    //locations2[p->getID()] = p;
    //locations2[p->getID()]->printDetails();

    auto it = locations.find(p->getID());

    if(it!=locations.end()){
        grid->updateCheckIn(p, (*it).second->getX(), (*it).second->getY());
        locations.insert(pair<int, Point*>(p->getID(), p));
    }

}


bool GPOs::loadLocations(const char* fileName){

    ifstream fin(fileName);
    if (! fin)
    {
        std::cerr << "Cannot open checkins file " << fileName << std::endl;
        return false;
    }

    cout << "HashTable: [ User_id -> location ] Loading from " << fileName << endl;

    int op, id;
    double x1, x2, y1, y2;
    int count = 0;
    Point* u;

    while (fin){
	
        fin >> op >> id >> x1 >> y1 >> x2 >> y2;
        if (! fin.good()) continue; // skip newlines, etc.
        u = new Point(x1, y1, id);

        //locations2[id] = u;
        locations.insert(pair<int, Point*>(id, u));

        grid->addCheckIn(u);

        ids.push_back(id);

        count ++;
        if(count%100000==0)
            cout << count << endl;
    }

    fin.close();
    cout << "Done! Num of users: " <<  count << endl;

    return true;
}



void GPOs::clearNextNN(){

    //	cout << "clearNextNN nextNNList size = " << nextNNList->size() << endl;

    while(!nextNNList->empty()) {
        delete nextNNList->back();
        nextNNList->pop_back();
        //		objects--;
    }

    //	cout << "clearNextNN nextNNList size = " << nextNNList->size() << " objects = " << objects << endl;

    delete nextNNList;

    //        nextNNList->clear();
    //        delete nextNNList;
    nextNNList = new vector<res_point*>();
    computedNN = returnedNN = finalNextNN = objects = 0;
    flagNextNN = true;
	
	pureNNexec = 0;

	delete incrVis;
	
}

double GPOs::estimateNearestDistance(double x, double y, int k){
    return grid->estimateNearestDistance(x,y,k);
}
