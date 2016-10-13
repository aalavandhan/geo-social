#include "../../headersMemory.h"

GPOs::GPOs (char* gridFileName, int numOfFiles){

    //pages = (int*) malloc(sizeof(int));
    //pages[0] = 0;

    kNNExecutions = 0;
    LocationExecutions = 0;
    NextNNExecutions = 0;
    RangeExecutions = 0;
	pureNNexec = 0;

    totalCPUTime = totalTime = 0.0;

    grid = new Grid;
    //  grid->loadFromFile(gridFileName);

    //locations = new map<int, Point*>(); ----

    //loadLocations(gridFileName);
	loadLocations(gridFileName, numOfFiles);

    objects = 0;

    // nextNN without incremental approach of NN
    computedNN = returnedNN = finalNextNN = 0;
    nextNNList = new vector<res_point*>();
    flagNextNN = true;
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
    /*
    Tables implementation

    if(id > 0 && id < DATASET_SIZE){
        Point *tmp = locations2[id];
        if(tmp->getID() == id){
            result[0]= tmp->getX();
            result[1]= tmp->getY();
        }
        else
            result[0] = -1000;
    }
    else
        result[0] = -1000;
*/
    //	Map implementation

    it = locations.find(id);

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




res_point* GPOs::getNextNN(double x, double y){
	clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();
	
	if(pureNNexec == 0)
		incrVis = new IncrVisitor(x, y);
		
	grid->getNextNN(*incrVis, 1);	

	pureNNexec++;
	
	endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

	return incrVis->getNext();
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
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    RangeExecutions++;

    vector<res_point*>* res = grid->getRange(x, y, radius);

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

    //    cout << "Num of users (GPOs) in radius = "<<radius<<" are =" << res->size() << endl;
    return res;
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


    it = locations.find(p->getID());

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

bool GPOs::loadLocations(const char* fileName, int numOfFiles){

	int fileNumber = 0;int count = 0;
	
	while(fileNumber < numOfFiles){
	
		stringstream po;
		po<<fileName<<"_"<<fileNumber<<".txt";
		string tmp = po.str();
		char *sentBuffer = new char[tmp.size()+1];
		sentBuffer[tmp.size()]='\0';
		memcpy(sentBuffer,tmp.c_str(),tmp.size());
		
		ifstream fin(sentBuffer);
		if (! fin)
		{
			std::cerr << "Cannot open checkins file " <<fileName<<"_"<<fileNumber<<".txt"<<std::endl;
			return false;
		}

		cout << "Loading from " << fileName <<"_"<<fileNumber<<".txt ...";

		int op, id;
		double x1, x2, y1, y2;
		
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
		}
		fileNumber++;
		fin.close();
		cout<<" Loaded till "<<count<<" users"<<endl;
		
	}
	if(numOfFiles == 1){
		DATASET_SIZE = count;
	}
	cout << "Done! Total number of checkins: " <<  count << endl;
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
