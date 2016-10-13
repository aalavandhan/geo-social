#include "../../headersDecentralized.h"

Graph_D::Graph_D(int port, char* host) {

	portno = port;

    	server = gethostbyname(host);
    	if (server == NULL)
        	cout << "ERROR, no such host" << endl;

    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    	serv_addr.sin_port = htons(portno);

	//cout << x;
	areFriendsExecutions = getFriendsExecutions = 0;
	totalCPUTime = totalTime = 0.0;
}


double Graph_D::getTotalCPUTime(){
        return totalCPUTime;
}

double Graph_D::getTotalTime(){
        return totalTime;
}


int Graph_D::getAreFriendsExecutions(){
	return areFriendsExecutions;
}


int Graph_D::getGetFriendsExecutions(){
	return getFriendsExecutions;
}


void Graph_D::getFriends(int id, int*& friendsF ,unsigned int& numOfFriends){
	clock_t startC, endC;
	struct timeval start, end;
        gettimeofday(&start, NULL);
	startC = clock();
	getFriendsExecutions++;

    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0) 
        	cout << "ERROR opening socket" << endl;

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        	cout << "ERROR connecting" << endl;

    	bzero(buffer,256);

	stringstream x;
        x << "getFriends_" << id;
        string tmp = x.str();
	
	int streamSize = tmp.size() + 10;
    stringstream fixedLengthStream;
    fixedLengthStream << setfill('0') << setw(9) << streamSize << "_";
    tmp = fixedLengthStream.str() + tmp ;
    
    char *sentBuffer = new char[tmp.size()+1];
    sentBuffer[tmp.size()]=0;
    memcpy(sentBuffer,tmp.c_str(),tmp.size());
    void *p = sentBuffer;
    while (streamSize > 0) {
	    int bytes_written = write(sockfd, p, streamSize);
	    if (bytes_written <= 0) {
		cout<<"ERROR writing to socket";
	    }
	    streamSize -= bytes_written;
	    p += bytes_written;
    }
    
    //cout<<"QUERY SENT"<<endl;
    bzero(buffer,256);

    n = read(sockfd,buffer,255);    
    if (n < 0)
	 cout << "ERROR reading from socket" << endl;
    
    //cout<<"buffer = "<<buffer<<endl;
    
    char bytesCount[10];
    strncpy ( bytesCount,buffer, 9 );
    bytesCount[9]='\0';
    int bytes_read = atoi(bytesCount);
    
    int bigBufSize = bytes_read-10;
    char bigBuffer[bigBufSize+1];
    bzero(bigBuffer,bigBufSize+1);
    strncpy ( bigBuffer, &buffer[10], n-10 );

   // cout<<"BIGbuffer = "<<bigBuffer<<endl;

    //cout <<"total bytes to be read: "<<bytes_read<<endl;
    bytes_read= bytes_read - n;
    //cout <<"total bytes left: "<<bytes_read<<endl;
    bzero(buffer,256);
    int arrayPos = n-10;
    while(bytes_read > 0){
      //cout <<"bytes to be read: "<<bytes_read<<endl;
      //cout<<"BIGbuffer = "<<bigBuffer<<endl;
	  n = read(sockfd,&bigBuffer[arrayPos],bigBufSize-arrayPos);
	  if (n < 0){
	      cout << "ERROR reading from socket" << endl;
	  }
	  //cout<<"BIGbuffer after copy = "<<bigBuffer<<endl;
	  arrayPos+=n;
	  bytes_read = bytes_read - n;
	  //cout <<"bytes to be read: "<<bytes_read<<endl;
     }
     bigBuffer[bigBufSize]='\0';

  // cout<<"BIGbuffer = "<<bigBuffer<<endl;
  // cout<<"BIGbuffer size = "<<sizeof(bigBuffer)<<endl;
   
    User* user ;
    Pair* friends; 
    int totalUserWeight = 0; int fid,fw;
    
    
	char *split;
	char *saveptr;
	int i =0;bool isID = true;int pos=0;
	split = strtok_r(bigBuffer, "_",&saveptr);

                while(split != NULL){
//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
		   if(strlen(split) > 0){
			if(i == 0){
				numOfFriends = atoi(split);
				user = new User(numOfFriends, id);
				friends = (Pair*)malloc(sizeof(Pair)*numOfFriends);
			}
			else{
			  if(isID){
				fid = atoi(split);
				isID=false;
			  }
			  else{
			        fw = atof(split);
				totalUserWeight+=fw;
				Pair * tmp = new Pair(fid, fw);
				*(friends+pos) = *tmp;
				isID=true;
				fid=0;
				fw=0;
				pos++;
			  }
				
			}
		   }
		   split = strtok_r(NULL, "_",&saveptr);
		   i++;
		}
		
	user->setFriends(friends, numOfFriends);
	user->setTotalWeight(totalUserWeight);	
		
	friendsF = (int*)user;
		
	close(sockfd);
	endC = clock();
	totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
}


bool Graph_D::areFriends(int user1, int user2){
	clock_t startC, endC;
	struct timeval start, end;
        gettimeofday(&start, NULL);
	startC = clock();

	areFriendsExecutions++;

    	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    	if (sockfd < 0) 
        	cout << "ERROR opening socket" << endl;
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        	cout << "ERROR connecting" << endl;
    
    	bzero(buffer,256);

	stringstream x;
        x << "areFriends_" << user1 <<"_" << user2;
        string tmp = x.str();
	
	int streamSize = tmp.size() + 10;
    stringstream fixedLengthStream;
    fixedLengthStream << setfill('0') << setw(9) << streamSize << "_";
    tmp = fixedLengthStream.str() + tmp ;
   // cout <<"sent string size= "<< tmp.size() << endl;
  //  cout <<"sent string is = "<<tmp<<endl;
    
    char *sentBuffer = new char[tmp.size()+1];
    sentBuffer[tmp.size()]=0;
    memcpy(sentBuffer,tmp.c_str(),tmp.size());
    void *p = sentBuffer;
    while (streamSize > 0) {
	    int bytes_written = write(sockfd, p, streamSize);
	    if (bytes_written <= 0) {
		cout<<"ERROR writing to socket";
	    }
	    streamSize -= bytes_written;
	    p += bytes_written;
    }
    
    bzero(buffer,256);

    n = read(sockfd,buffer,1);
    
    if (n < 0)
	 cout << "ERROR reading from socket" << endl;

	if(buffer[0] == '1'){
		close(sockfd);

		endC = clock();
		totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
		gettimeofday(&end, NULL);
		totalTime += util.print_time(start, end);
		return true;
	}
	else{
		close(sockfd);

		endC = clock();
		totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
		gettimeofday(&end, NULL);
		totalTime += util.print_time(start, end);
		return false;
	}
}


int Graph_D::getUserDegree(int id){
return 0;
}
