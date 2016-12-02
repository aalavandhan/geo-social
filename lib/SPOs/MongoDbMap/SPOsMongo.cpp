//#include "../../headersMongoDb.h"
#include "mongo/client/dbclient.h"
using namespace mongo;
#include "../../headers.h"
#include "SPOsMongo.h"

SPOsMongo::SPOsMongo(string server, string collection){
	c.connect(server);
	coll = collection;

	getFriendsExecutions = 0;
	areFriendsExecutions = 0;

	totalTime = totalCPUTime = 0.0;
}

SPOsMongo::~SPOsMongo(){}


double SPOsMongo::getTotalCPUTime(){
	return totalCPUTime;
}

double SPOsMongo::getTotalTime(){
	return totalTime;
}


void SPOsMongo::addFriendship(int user1, int user2){

	/*	BSONObjBuilder user1_builder;
	user1_builder << "friends" << user2;
	BSONObj user1_friends = user1_builder.obj();

	c.update(coll, BSON("user" << user1), BSON("$push" << user1_friends));
*/

	//cout << "Addfriendship started" << endl;

	int *friends1, *friends2; 
	unsigned int size1 = 0, size2 = 0 ;


	getFriends(user1, friends1, size1);
	int* tmp1;	
	if(size1>0)
	tmp1 = (int*) realloc(friends1, (size1+1)*sizeof(int));
	else
	tmp1 = (int*) malloc((size1+1)*sizeof(int));	
	if(tmp1 != NULL){
		friends1 = tmp1;
		size1++;
		if(size1 > 2) // ????????????????????????????? check addtosortedlist
		util.addToSortedList(friends1, size1, user2);
		else
		friends1[0]=user2;	
	}
	setFriends(user1, friends1, size1);
	

	
	getFriends(user2, friends2, size2);
	int* tmp2;	
	if(size2>0)
	tmp2 = (int*) realloc(friends2, (size2+1)*sizeof(int));
	else
	tmp2 = (int*) malloc((size2+1)*sizeof(int));	
	if(tmp2 != NULL){
		friends2 = tmp2;
		size2++;
		if(size2 > 2) // ????????????????????????????? check addtosortedlist
		util.addToSortedList(friends2, size2, user1);
		else
		friends2[0]=user1;
	}
	setFriends(user2, friends2, size2);
	
}


void SPOsMongo::setFriends(int id, int*& friends, unsigned int& numOfFriends){

	//cout << "user: " << id << " set size = " << numOfFriends << endl;
	
	
	BSONArrayBuilder ba;
	for(unsigned int i = 0; i < numOfFriends; i++)
	ba.append(friends[i]);
	//ba.arr();
	
	/*vector< BSONElement > f(numOfFriends);
	for(int i = 0; i < numOfFriends; i++)
		f[i].Val(friends[i]);
	*/
	BSONObjBuilder b;
	b << "friends" << ba.arr();
	BSONObj q = b.obj();

	c.update(coll, BSON("user" << id), BSON("$set" << q));

	//cout << "finished" << endl;

}


bool SPOsMongo::areFriends(int user1, int user2){

	clock_t startC, endC;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	startC = clock();

	areFriendsExecutions++;

	BSONObjBuilder b;
	b << "user" << user1 << "friends" << user2;
	BSONObj q = b.obj();
	auto_ptr<DBClientCursor> cursor = c.query(coll, q );


	endC = clock();
	totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
	gettimeofday(&end, NULL);
	totalTime += util.print_time(start, end);

	if( cursor->more() )
	return true;
	
	return false;
}

void SPOsMongo::getFriends(int id, int*& friends, unsigned int& numOfFriends){

	clock_t startC, endC;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	startC = clock();

	getFriendsExecutions++;

	auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "user" << id ) );

	if( cursor->more() ){
		BSONObj p = cursor->next();
		vector< BSONElement > x = p.getField("friends").Array();
		int size = x.size();
		numOfFriends = size;
		friends = (int*) malloc(sizeof(int)*size);
		
		for(int i=0; i< size; i++){
			friends[i] = x[i].Int();
		}
	}

	endC = clock();
	totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
	gettimeofday(&end, NULL);
	totalTime += util.print_time(start, end);
}

int SPOsMongo::getUserDegree(int id ){
	clock_t startC, endC;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	startC = clock();

	auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "user" << id ) );
	int size = 0;
	if( cursor->more() ){
		BSONObj p = cursor->next();
		vector< BSONElement > x = p.getField("friends").Array();
		size = x.size();
	}

//TO BE IMPLEMENTED
	endC = clock();
	totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
	gettimeofday(&end, NULL);
	totalTime += util.print_time(start, end);
	
	return size;
}

void SPOsMongo::printTriangles( int id, vector<int> friendList){
	int num = 0;
	
	cout << "PRINTING TRIANGLES of user "<<id<<endl;
	for (unsigned int i = 0; i< friendList.size();i++){
		for (unsigned int j = i; j< friendList.size();j++){
			if(areFriends(friendList.at(i),friendList.at(j))){
				cout << "{"<<friendList.at(i)<<" , "<<friendList.at(j)<<"}" << " ";
				num ++;
			}
		}
	}	
	cout<<"Total triangles = "<<num<<endl;
}

int SPOsMongo::getAreFriendsExecutions(){
	return areFriendsExecutions;
}

int SPOsMongo::getGetFriendsExecutions(){
	return getFriendsExecutions;
}
