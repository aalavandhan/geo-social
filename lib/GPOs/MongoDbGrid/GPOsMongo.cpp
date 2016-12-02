//#include "../../headersMongoDb.h"

#include "mongo/client/dbclient.h"
using namespace mongo;
#include "../../headers.h"
#include "GPOsMongo.h"

GPOsMongo::GPOsMongo(string server, string collection){
	c.connect(server);
	coll = collection;

	computedNN = returnedNN = finalNextNN = 0;
	nextNNList = new vector<res_point*>();

	flagNextNN = true;

	kNNExecutions = 0;
	LocationExecutions = 0;
	NextNNExecutions = 0;
	RangeExecutions = 0;

	totalTime = totalCPUTime = 0.0;
	measurekNNTime = true;
}

GPOsMongo::~GPOsMongo(){}


void GPOsMongo::setMeasurekNNTime(bool set){

	measurekNNTime = set;
}


double GPOsMongo::getTotalCPUTime(){
	return totalCPUTime;
}

double GPOsMongo::getTotalTime(){
	return totalTime;
}


vector<res_point*>* GPOsMongo::getRangeSortedId(double x, double y, double radius){
	// to be implemented
	return NULL;
}


void GPOsMongo::getLocation(int id, double* result){
	clock_t startC, endC;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	startC = clock();

	auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "user" << id ) );
	vector< BSONElement > v;
	LocationExecutions++;

	while( cursor->more() ) {
		BSONObj p = cursor->next();
		v = p.getField("loc").Array();
		result[0] = v[0].Double();
		result[1] = v[1].Double();

		endC = clock();
		totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
		gettimeofday(&end, NULL);
		totalTime += util.print_time(start, end);
		return;
	}

	result[0] = -1000;
	result[1] = -1000;
	endC = clock();
	totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
	gettimeofday(&end, NULL);
	totalTime += util.print_time(start, end);

	return;
}


void GPOsMongo::updateCheckin(int id, double x, double y){

	c.remove(coll, QUERY("user" << id));

	BSONObjBuilder b;
	b.append("user", id);
	b.append("loc,", BSON_ARRAY( x << y ));
	BSONObj p = b.obj();

	c.insert(coll,p);



	/*
	BSONObjBuilder x_builder;
	x_builder << "loc.0" << x;
	BSONObj locationX = x_builder.obj();

	c.update(coll, BSON("user" << id), BSON("$set" << locationX));

	BSONObjBuilder y_builder;
	y_builder << "loc.1" << y;
	BSONObj locationY = y_builder.obj();

	c.update(coll, BSON("user" << id), BSON("$set" << locationY));
*/
}


vector<res_point*>* GPOsMongo::getkNN(double x, double y, int k){
	clock_t startC, endC;
	struct timeval start, end;

	if(measurekNNTime){
		gettimeofday(&start, NULL);
		startC = clock();
	}

	auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "loc" << BSON("$near"
	<< BSON_ARRAY( x << y ) ) ), k );
	vector<res_point*>* res = new vector<res_point*>();


	kNNExecutions++;

	while( cursor->more() ) {
		res_point* u = new res_point;
		BSONObj p = cursor->next();
		u-> id = p.getField("user").Int();
		vector< BSONElement > v = p.getField("loc").Array();
		u->x = v[0].Double();
		u->y = v[1].Double();
		u->dist = util.computeMinimumDistance(u->x, u->y, x, y);
		res->push_back(u);
	}

	if(measurekNNTime){
		endC = clock();
		totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
		gettimeofday(&end, NULL);
		totalTime += util.print_time(start, end);
	}

	return res;
}



vector<res_point*>* GPOsMongo::getRange(double x, double y, double radius){
	clock_t startC, endC;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	startC = clock();

	BSONObjBuilder b;
	b << "center" << BSON_ARRAY( x << y ) << "radius" << radius;
	BSONObj center = b.obj();
	RangeExecutions++;

	auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "loc" << BSON(
	"$within" << BSON("$center" << center) ) ) );

	vector<res_point*>* res = new vector<res_point*>();
	while( cursor->more() ) {
		res_point* u = new res_point;	    	
		BSONObj p = cursor->next();
		u-> id = p.getField("user").Int();
		vector< BSONElement > v = p.getField("loc").Array();			
		u->x = v[0].Double();
		u->y = v[1].Double();	
		u->dist = util.computeMinimumDistance(u->x, u->y, x, y);		
		res->push_back(u);
	}

	endC = clock();
	totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
	gettimeofday(&end, NULL);
	totalTime += util.print_time(start, end);

	return res;
}


res_point* GPOsMongo::getNextNN(double x, double y, int incrStep){
	clock_t startC, endC;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	startC = clock();

	

	if(computedNN <= returnedNN && flagNextNN){
		NextNNExecutions++;
		// compute the next NN]
		computedNN+=incrStep;
		setMeasurekNNTime(false);
		vector<res_point*>* kNN = getkNN(x, y, computedNN); // do not measure the time needed ---- here is the time
		int size = kNN->size();

		for(int i = returnedNN; i < size; i++){
			nextNNList->push_back(util.copy((*kNN)[i]));
		}

		while(!kNN->empty()) {
			delete kNN->back();
			kNN->pop_back();
		}
		delete kNN;

		int newNNsize = nextNNList->size();
		if(computedNN > newNNsize){ // no more!
			flagNextNN = false;
			computedNN = newNNsize;
		}

	}

	//	if(computedNN > returnedNN && flagNextNN){
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

void GPOsMongo::clearNextNN(){

	while(!nextNNList->empty()) {
		delete nextNNList->back();
		nextNNList->pop_back();
	}
	delete nextNNList;	


	//	nextNNList->clear();	
	//	delete nextNNList;
	nextNNList = new vector<res_point*>();
	computedNN = returnedNN = finalNextNN = 0;
	flagNextNN = true;
}

int GPOsMongo::getkNNExecutions(){
	return kNNExecutions;
}

int GPOsMongo::getLocationExecutions(){
	return LocationExecutions;
}

int GPOsMongo::getNextNNExecutions(){
	return NextNNExecutions;
}

int GPOsMongo::getRangeExecutions(){
	return RangeExecutions;
}


set<res_point*, res_point_ascending_id>* GPOsMongo::getSetRange(double x, double y, double radius){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    RangeExecutions++;
	//to be implemented
    set<res_point*, res_point_ascending_id>* res =  new set<res_point*, res_point_ascending_id>();//grid->getSetRange(x, y, radius);

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

    //    cout << "Num of users (GPOsMongo) in radius = "<<radius<<" are =" << res->size() << endl;
    return res;
}

double GPOsMongo::estimateNearestDistance(double x, double y, int k){
    //to be implemented
	return 0;
}

