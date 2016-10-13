#include "../../headersMemory.h"

Graph::Graph(){}

Graph::Graph(int numOfUsers){
	V = numOfUsers;
	graph = (User*)malloc(sizeof(User)*numOfUsers); 

}

User* Graph::getUser(int user){
      return graph+user;      
}

void Graph::getFriends(int user, int*& friends, unsigned int &numOfFriends){

	friends = (int*)(graph+user);
	numOfFriends = graph[user].getFriendsSize();
}

bool Graph::areFriends(int id1, int id2){
  
	User* user1 = graph+id1;
	int numOfFriends = user1->getFriendsSize();
	Pair* pairFriends = user1->getFriends();
	for(int num =0; num <numOfFriends;num++){
	    //cout<<"\tid = "<<(pairFriends[num]).id<<" weight = "<<(pairFriends[num]).weight<<endl;
	    if(id2 == (pairFriends[num]).id)
	      return true;
	}

	return false;
}

double Graph::load(const char* file){   //returns the total weight of the social edges
	
	double totalWeight = 0;
	
	ifstream fin(file);	
	if (! fin){
		cout << "Cannot open Social graph file " << file << endl;
		return -1;
	}

	int id, size;
	int times = 0;
	int fid;
	double weight;

	int totalFriends = 0;
	double totalUserWeight;

	cout << "Loading the Social Graph from " << file << endl; 	
	User* user;
	while(fin ){ 
		
		fin >> id >> size;
		if (! fin.good()){
			cout << "fin is not good: id = " << id << endl;
			continue;
		}

       // cout << times << "\t id = " << id << " # friends = " << size << endl;

		user = new User(size, id);
		totalUserWeight = 0;

		Pair* friends = (Pair*)malloc(sizeof(Pair)*size);
		totalFriends+=sizeof(pair<int, double>)*size;
		for(int i = 0; i<size; i++){
			fin >> fid >> weight;
	//		cout << fid << "\t" << weight << endl;
			Pair * tmp = new Pair(fid, weight);
			*(friends+i) = *tmp;
			totalUserWeight+=weight;
			totalWeight+=weight;
		}

		user->setFriends(friends, size);
		user->setTotalWeight(totalUserWeight);
		*(graph+id) = *user;

		times++;

		if(times%100000 == 0)
			   cout << times << endl;
	}
	fin.clear();
	fin.close();
	cout << "Done!" << endl;
	cout << "totalFriends = " << (totalFriends/(1024*1024)) << "MB" << endl;
	return totalWeight;
}

int Graph::getUserDegree(int id){
return 0;
}


void Graph::printGraph(){

	for(int i=0; i<V; i++){
		graph[i].printUser();
	}
}
