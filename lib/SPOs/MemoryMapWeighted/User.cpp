#include "../../headersMemory.h"

User::User(int _size, int _id){
	id = _id;
	size = _size;
	friends = (Pair*)malloc(sizeof(Pair)*size);
}

User::~User(){
	
	delete friends;

}

Pair* User::getFriends(){             	
/*	int *t = (int*)malloc(sizeof(int)*size);	
	
	for (int i = 0; i < size; i++){
		*(t+i) = list[i];
	}
*/
	return friends;
}

    
int User::getFriendsSize() {
	return size;
}


void User::setTotalWeight(double weight){
     totalWeight = weight;
}

double User::getTotalWeight(){ 
       return totalWeight;
}


void User::setFriends(Pair* _friends, int _size){

	if(_size > 0){
		for (int i = 0; i < size; i++){
			*(friends+i) = _friends[i];
		}
		free(_friends);
	}
}

void User::printUser(){
	cout << "Id: "<< id << "\t #friends: " << size << " Friends:";
	for(int i = 0; i < size; i++){
		cout <<  "(" << friends[i].id << "," << friends[i].weight << ") ";
	}
	cout << endl;
}
