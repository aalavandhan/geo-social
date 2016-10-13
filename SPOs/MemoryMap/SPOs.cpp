#include "../../headersMemory.h"

SPOs::SPOs() { 
    areFriendsExecutions = getFriendsExecutions = 0;
    totalCPUTime = totalTime = 0.0;
}

double SPOs::getTotalCPUTime(){
    return totalCPUTime;
}

double SPOs::getTotalTime(){
    return totalTime;
}

int SPOs::getAreFriendsExecutions(){
    return areFriendsExecutions;
}

int SPOs::getGetFriendsExecutions(){
    return getFriendsExecutions;
}

/*
loads the social graph into a map/hashtable containing
(id, value) pair. The "value" data structure stores the
ids of all friends of a given user.
*/
int SPOs::load(const char* file){

    ifstream fin(file);
    if (!fin){
        cout << "Cannot open Social graph file " << file << endl;
        return -1;
    }

    int id, size;
    unsigned int times = 0;
    int totalFriends = 0;
    cout << "Loading the Social Graph from " << file << endl;
    Value* entry;
    while(fin){ //NUMOFUSERS

        fin >> id >> size;
        if (! fin.good()){
            // cout << "fin is not good: id = " << id << endl;
            continue;
        }

        entry = new Value(size, id);

        int* list = (int*) malloc(sizeof(int)*size);
        totalFriends+=sizeof(int)*size;
        for(int i = 0; i<size; i++){
            fin >> list[i];
        }
        entry->setList(list, size);

        //ids.push_back(id);

        hashTable.insert(pair<int, Value*>(id, entry));

        times++;
        if(times%1000000 == 0)
            cout << times << endl;
    }
    fin.clear();
    fin.close();
    cout << times << endl;
    cout << "Done!" << endl;
    cout << "totalFriends = " << (totalFriends/(1024)) << "KB" << endl;
    return 0;
}

int SPOs::load(const char* file, int count){

	unsigned int times = 0;
	int totalFriends = 0;
	int fileNumber = 0;
	while(fileNumber < count)	{
		
		stringstream po;
		po<<file<<"_"<<fileNumber<<".txt";
		string tmp = po.str();
		char *sentBuffer = new char[tmp.size()+1];
		sentBuffer[tmp.size()]='\0';
		memcpy(sentBuffer,tmp.c_str(),tmp.size());
		
		ifstream fin(sentBuffer);
		if (!fin){
			cout << "Cannot open Social graph file " << tmp << endl;
			return -1;
		}

		int id, size;

		cout << "Loading the Social Graph from " << tmp <<" ... ";
		Value* entry;
		while(fin){ //NUMOFUSERS

			fin >> id >> size;
			if (! fin.good()){
				// cout << "fin is not good: id = " << id << endl;
				continue;
			}

			entry = new Value(size, id);

			int* list = (int*) malloc(sizeof(int)*size);
			totalFriends+=sizeof(int)*size;
			for(int i = 0; i<size; i++){
				fin >> list[i];
			}
			entry->setList(list, size);

			//ids.push_back(id);

			hashTable.insert(pair<int, Value*>(id, entry));

			times++;
			
		}
		fin.clear();
		fin.close();
		cout<<" Loaded yet: "<< times<<"\n";
		fileNumber++;
	}
    cout << times << endl;
    cout << "Done!" << endl;
    cout << "totalFriends = " << (totalFriends/(1024)) << "KB" << endl;
    return 0;
}

//void SPOs::addFriendship(int user1, int user2){
//    // add user1 to the list of user2
//    //Value *user2_list = hashTable2[user2];
//    Value *user2_list = hashTable.find(user2)->second;
//    //	cout << "addFriendship one start user = " << user2 << endl;
//    int * tmp2 = (int*) realloc(user2_list->list, (user2_list->size+1)*sizeof(int));

//    if(tmp2 != NULL){
//        user2_list->list = tmp2;
//        user2_list->size = user2_list->size+1;
//        util.addToSortedList(user2_list->list, user2_list->size, user1);
//    }
//    else
//        cout << "Error on memory allocation" << endl;

//    //	cout << "addFriendship one finished" << endl;

//    // add user2 to the list of user1
//    //Value *user1_list = hashTable2[user1];
//    Value *user1_list = hashTable.find(user1)->second;
//    //	cout << "addFriendship for user" << user1 << endl;
//    int * tmp1 = (int*) realloc(user1_list->list, (user1_list->size+1)*sizeof(int));
//    //	cout << "memory located" << endl;
//    if(tmp1 != NULL){
//        user1_list->list = tmp1;
//        user1_list->size = user1_list->size+1;
//        util.addToSortedList(user1_list->list, user1_list->size, user2);
//    }
//    else
//        cout << "Error on memory allocation" << endl;
//    //	cout << "addFriendship two finished" << endl;
//}


//int SPOs::loadfriends(const char* fileName){

//    FILE *file;
//    int id, size;
//    int times = 0;


//    file = fopen(fileName,"r");
//    cout << "Loading the Social Graph from " << file << endl;
//    if(file==NULL)
//        cout << "Cannot open Social graph file " << file << endl;

//    cout << "Loading the Social Graph from " << fileName << endl;
//    int totalFriends = 0;
//    while(!feof(file)){ //NUMOFUSERS
//        //fin >> id >> size;
//        fscanf(file, "%d %d", &id, &size);

//        Value* entry = new Value(size, id);

//        int* list = (int*) malloc(sizeof(int)*size);
//        totalFriends+=sizeof(int)*size;
//        for(int i = 0; i<size; i++){
//            fscanf(file, " %d", &(list[i]));
//            //fin >> list[i];
//        }
//        //cout << "test : " << list[1] << endl;

//        entry->setList(list, size);

//        hashTable2[id] = entry;
//        //hashTable.insert(pair<int, Value*>(id, entry));
//        //delete list;

//        times++;
//        if(times%100000 == 0)
//            cout << times << endl;
//    }
//    fclose(file);
//    cout << "Done!" << endl;
//    cout << "totalFriends = " << (totalFriends/(1024)) << "KB" << endl;
//    return 0;
//}


/*
returns the size of the list of friends of a given user (id)
as the integer "numOfFriends" along with the list in integer
array "friends"
*/
void SPOs::getFriends(int id, int*& friends, unsigned int &numOfFriends){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();
    getFriendsExecutions++;

    Value* v = NULL;
    map<int, Value*>::iterator it = hashTable.find(id);
    if(it != hashTable.end()){
        //element found;
        v = it->second;
    }
    else {
        numOfFriends = 0;
        friends = (int*)malloc(sizeof(int)*numOfFriends);
    }

    if(v!=NULL){
        numOfFriends = v->getListSize();
        int* tmp = v->getList();
        friends = (int*)malloc(sizeof(int)*numOfFriends);

        for(unsigned int i=0; i< numOfFriends; i++){
            friends[i] = tmp[i];
        }
        free(tmp);
    }

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);
}

/*
iterates through the list of friends of user1 to find
user2. Returns boolean variable denoting the same.
*/
bool SPOs::areFriends(int user1, int user2){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    areFriendsExecutions++;

    Value* v = NULL;
    map<int, Value*>::iterator it = hashTable.find(user1);
    if(it != hashTable.end()){
        //element found;
        v = it->second;
    }

    if(v != NULL){
        int size = v->getListSize();
        int* f = v->getList();

        for(int j = 0; j < size; j++){
            if (f[j] == user2){
                endC = clock();
                totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
                gettimeofday(&end, NULL);
                totalTime += util.print_time(start, end);
                return true;
            }
        }
        free(f);
    }

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);
    return false;
}

//double SPOs::computeDensity(Group G){

//}
void SPOs::printTriangles( int id, vector<int> friendList){
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
	cout<<" | Total triangles = "<<num<<endl;
}

int SPOs::getUserDegree(int id){
	int degree = 0;
	Value* v = NULL;
    map<int, Value*>::iterator it = hashTable.find(id);
    if(it != hashTable.end()){
        //element found;
        v = it->second;
		if(v!=NULL){
        degree = v->getListSize();
		}
    }
	
	return degree;
}