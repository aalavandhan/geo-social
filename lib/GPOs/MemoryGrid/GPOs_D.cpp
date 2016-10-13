#include "../../headersDecentralized.h"

GPOs_D::GPOs_D(int port, char* host) {

    portno = port;

    server = gethostbyname(host);
    if (server == NULL)
        cout << "ERROR, no such host" << endl;

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);


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

GPOs_D::~GPOs_D(){}


void GPOs_D::setMeasurekNNTime(bool set){

    measurekNNTime = set;
}


double GPOs_D::getTotalCPUTime(){
    return totalCPUTime;
}

double GPOs_D::getTotalTime(){
    return totalTime;
}


vector<res_point*>* GPOs_D::getRangeSortedId(double x, double y, double radius){

    // to be implemented
    return NULL;
}


void GPOs_D::getLocation(int id, double* result){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cout << "ERROR opening socket" << endl;

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        cout << "ERROR connecting" << endl;
    
    bzero(buffer,256);

    stringstream x;
    x << "getLocation_" << id;
    string tmp = x.str();
    int streamSize = tmp.size() + 10;
    stringstream fixedLengthStream;
    fixedLengthStream << setfill('0') << setw(9) << streamSize << "_";
    tmp = fixedLengthStream.str() + tmp ;
    //cout <<"sending string: "<<tmp<<" -- of size = "<<tmp.size()<<endl;
    char *sentBuffer = new char[tmp.size()+1];
    sentBuffer[tmp.size()]='\0';
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

    //cout<<"BIGbuffer = "<<bigBuffer<<endl;

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
	  arrayPos += n;
	  bytes_read = bytes_read - n;
	  //cout <<"bytes to be read: "<<bytes_read<<endl;
     }
     bigBuffer[bigBufSize]='\0';

   // cout<<"BIGbuffer = "<<bigBuffer<<endl;
   // cout<<"BIGbuffer size = "<<sizeof(bigBuffer)<<endl;

    //        cout << "Buffer = " << buffer << endl;

    char* split = strtok(bigBuffer, "_");
    result[0] = atof(split);

    split = strtok(NULL, "_");
    result[1] = atof(split);

    close(sockfd);

    /*	auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "user" << id ) );
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
*/

    endC = clock();
    totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
    gettimeofday(&end, NULL);
    totalTime += util.print_time(start, end);

    return;
}



vector<res_point*>* GPOs_D::getkNN(double x, double y, int k){
    clock_t startC, endC;
    struct timeval start, end;

    if(measurekNNTime){
        gettimeofday(&start, NULL);
        startC = clock();
    }    
    kNNExecutions++;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cout << "ERROR opening socket" << endl;


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        cout << "ERROR connecting" << endl;
    

    stringstream po;
    po.precision(15);
    po << "getkNN_" << x << "_" << y << "_" << k;
    
    string tmp = po.str();
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

   //cout<<"BIGbuffer = "<<bigBuffer<<endl;

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

    //cout<<"BIGbuffer = "<<bigBuffer<<endl;
    //cout<<"BIGbuffer size = "<<sizeof(bigBuffer)<<endl;
    
    vector<res_point*>* resultVec = new vector<res_point*>();
    
      int id;double xx,yy,dist;
      char *finalSplit;char *saveptr;
      int r=0;

      finalSplit = strtok_r(bigBuffer, "_", &saveptr);
      //cout<<"-spli 2 : "<<finalSplit<<endl;	
      while(finalSplit!=NULL){
	//cout<<"iteration number r ="<<r<<endl;
	if(r%4==0){
	  //cout<<"id ="<<atof(finalSplit)<<endl;
	  id=atoi(finalSplit);
	}
	else if(r%4==1){
	  //cout<<"x ="<<atof(finalSplit)<<endl;
	  xx=atof(finalSplit);
	}
	else if(r%4==2){
	 // cout<<"y ="<<atof(finalSplit)<<endl;
	  yy=atof(finalSplit);
	}else if(r%4==3){
	  //cout<<"dist ="<<atof(finalSplit)<<endl;
	  dist=atof(finalSplit);
	  //cout<<"inserting...";
	  res_point* u = new res_point;
	  u->id = id;
	  u->x = xx;
	  u->y = yy;
	  u->dist = dist;
	  resultVec->push_back(u);
	  //cout<<"inserted"<<endl;
	}
	finalSplit = strtok_r(NULL, "_", &saveptr);
	r++; 
      }
      
    cout<<"size of result vector = " <<resultVec->size()<<endl;
    close(sockfd);

    /*
    auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "loc" << BSON("$near" << BSON_ARRAY( x << y ) ) ), k );

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
*/
    if(measurekNNTime){
        endC = clock();
        totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
    }

    cout<<"closing function"<<endl;
    return resultVec;
}



vector<res_point*>* GPOs_D::getRange(double x, double y, double radius){
   clock_t startC, endC;
    struct timeval start, end;

    if(measurekNNTime){
        gettimeofday(&start, NULL);
        startC = clock();
    }    
    kNNExecutions++;

    cout<<"opening socket... ...";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cout << "ERROR opening socket" << endl;

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        cout << "ERROR connecting" << endl;
    cout<<"connected"<<endl;

    stringstream po;
    po.precision(15);
    po << "getRange_" << x << "_" << y << "_" << radius;
    
    string tmp = po.str();
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
    
    bzero(buffer,256);

    n = read(sockfd,buffer,255);    
    if (n < 0)
	 cout << "ERROR reading from socket" << endl;
    
    cout << " N IS =" <<n<<endl;
    
    if(n<10){
      cout<<"FUCKING N IS LESS THAN 10"<<endl;
      exit(-1);
    }
    
    cout<<"buffer = "<<buffer<<endl;
    
    char bytesCount[10];
    strncpy ( bytesCount,buffer, 9 );
    bytesCount[9]='\0';
    int bytes_read = atoi(bytesCount);
    
    int bigBufSize = bytes_read-10;
    char bigBuffer[bigBufSize+1];
    bzero(bigBuffer,bigBufSize+1);
    strncpy ( bigBuffer, &buffer[10], n-10 );

    cout<<"BIGbuffer = "<<bigBuffer<<endl;

    cout <<"total bytes to be read: "<<bytes_read<<endl;
    bytes_read= bytes_read - n;
    cout <<"total bytes left: "<<bytes_read<<endl;
    bzero(buffer,256);
    int arrayPos = n-10;
    while(bytes_read > 0){
      cout <<"bytes to be read: "<<bytes_read<<endl;
      cout<<"BIGbuffer = "<<bigBuffer<<endl;
	  n = read(sockfd,&bigBuffer[arrayPos],bigBufSize-arrayPos);
	  cout << " N IS NOW =" <<n<<endl;
	  cout << "BIGBUFSIzE  = "<<bigBufSize<<endl;
	  cout << "bigBufSize - arrayPos = " <<bigBufSize-arrayPos<<endl;
	  if (n < 0){
	      cout << "ERROR reading from socket" << endl;
	  }
	  cout<<"BIGbuffer after copy = "<<bigBuffer<<endl;
	  arrayPos+=n;
	  bytes_read = bytes_read - n;
	  cout <<"bytes to be read: "<<bytes_read<<endl;
     }
     bigBuffer[bigBufSize]='\0';

    cout<<"BIGbuffer = "<<bigBuffer<<endl;
    cout<<"BIGbuffer size = "<<sizeof(bigBuffer)<<endl;
    
    vector<res_point*>* resultVec = new vector<res_point*>();
    
      int id;double xx,yy,dist;
      char *finalSplit;char *saveptr;
      int r=0;

      finalSplit = strtok_r(bigBuffer, "_", &saveptr);
      //cout<<"-spli 2 : "<<finalSplit<<endl;	
      while(finalSplit!=NULL){
	//cout<<"iteration number r ="<<r<<endl;
	if(r%4==0){
	  //cout<<"id ="<<atof(finalSplit)<<endl;
	  id=atoi(finalSplit);
	}
	else if(r%4==1){
	  //cout<<"x ="<<atof(finalSplit)<<endl;
	  xx=atof(finalSplit);
	}
	else if(r%4==2){
	 // cout<<"y ="<<atof(finalSplit)<<endl;
	  yy=atof(finalSplit);
	}else if(r%4==3){
	  //cout<<"dist ="<<atof(finalSplit)<<endl;
	  dist=atof(finalSplit);
	  //cout<<"inserting...";
	  res_point* u = new res_point;
	  u->id = id;
	  u->x = xx;
	  u->y = yy;
	  u->dist = dist;
	  resultVec->push_back(u);
	  //cout<<"inserted"<<endl;
	}
	finalSplit = strtok_r(NULL, "_", &saveptr);
	r++; 
      }
      
    //cout<<"size of result vector = " <<resultVec->size()<<endl;
    cout<<"closing socket .... ...";
    close(sockfd);
  cout<<"closed!"<<endl;


    /*
    BSONObjBuilder b;
    b << "center" << BSON_ARRAY( x << y ) << "radius" << radius;
    BSONObj center = b.obj();
    RangeExecutions++;

    auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "loc" << BSON("$within" << BSON("$center" << center) ) ) );

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
*/
    if(measurekNNTime){
        endC = clock();
        totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
    }

    return resultVec;
}


set<res_point*, res_point_ascending_id>* GPOs_D::getSetRange(double x, double y, double radius){
clock_t startC, endC;
    struct timeval start, end;

    if(measurekNNTime){
        gettimeofday(&start, NULL);
        startC = clock();
    }    
    kNNExecutions++;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cout << "ERROR opening socket" << endl;


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        cout << "ERROR connecting" << endl;
    

    stringstream po;
    po.precision(15);
    po << "getSetRange_" << x << "_" << y << "_" << radius;
    
    string tmp = po.str();
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

   //cout<<"BIGbuffer = "<<bigBuffer<<endl;

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

    //cout<<"BIGbuffer = "<<bigBuffer<<endl;
    //cout<<"BIGbuffer size = "<<sizeof(bigBuffer)<<endl;
    
    set<res_point*, res_point_ascending_id>* resultSet = new set<res_point*, res_point_ascending_id>();
    
      int id;double xx,yy,dist;
      char *finalSplit;char *saveptr;
      int r=0;

      finalSplit = strtok_r(bigBuffer, "_", &saveptr);
      //cout<<"-spli 2 : "<<finalSplit<<endl;	
      while(finalSplit!=NULL){
	//cout<<"iteration number r ="<<r<<endl;
	if(r%4==0){
	  //cout<<"id ="<<atof(finalSplit)<<endl;
	  id=atoi(finalSplit);
	}
	else if(r%4==1){
	  //cout<<"x ="<<atof(finalSplit)<<endl;
	  xx=atof(finalSplit);
	}
	else if(r%4==2){
	 // cout<<"y ="<<atof(finalSplit)<<endl;
	  yy=atof(finalSplit);
	}else if(r%4==3){
	  //cout<<"dist ="<<atof(finalSplit)<<endl;
	  dist=atof(finalSplit);
	  //cout<<"inserting...";
	  res_point* u = new res_point;
	  u->id = id;
	  u->x = xx;
	  u->y = yy;
	  u->dist = dist;
	  
	  set<res_point*, res_point_ascending_id>::iterator it = resultSet->end();
	  resultSet->insert(it,u);
	  //cout<<"inserted"<<endl;
	}
	finalSplit = strtok_r(NULL, "_", &saveptr);
	r++; 
      }
      
    //cout<<"size of result vector = " <<resultSet->size()<<endl;
    close(sockfd);



    /*
    BSONObjBuilder b;
    b << "center" << BSON_ARRAY( x << y ) << "radius" << radius;
    BSONObj center = b.obj();
    RangeExecutions++;

    auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "loc" << BSON("$within" << BSON("$center" << center) ) ) );

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
*/
    if(measurekNNTime){
        endC = clock();
        totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
    }

    return resultSet;
}


res_point* GPOs_D::getNextNN(double x, double y, int incrStep){
    clock_t startC, endC;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    startC = clock();

    if(computedNN <= returnedNN && flagNextNN){
        NextNNExecutions++;

        // compute the next NN]
        computedNN+=incrStep;
        setMeasurekNNTime(false);
        vector<res_point*>* kNN = getkNN(x, y, computedNN); // do not measure the time needed -------------------------------------------- here is the time
        int size = kNN->size();

        for(int i = returnedNN; i < size; i++){
            nextNNList->push_back(util.copy((*kNN)[i]));
        }

        //		cout << "Start Deletion" << endl;
        int j = 0;
        while(!kNN->empty()) {
            //			cout << j << ") Deletion id" << kNN->back()->id << "\t x = " << kNN->back()->x << "\t y = " << kNN->back()->y << "\t dist = " << kNN->back()->dist << endl;
            delete kNN->back();
            kNN->pop_back();
            j++;
        }
        delete kNN;
        //cout << "End Deletion" << endl;


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


double GPOs_D::estimateNearestDistance(double x, double y, int k){
    clock_t startC, endC;
    struct timeval start, end;

    if(measurekNNTime){
        gettimeofday(&start, NULL);
        startC = clock();
    }    
    kNNExecutions++;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cout << "ERROR opening socket" << endl;


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        cout << "ERROR connecting" << endl;
    

    stringstream po;
    po.precision(15);
    po << "estimateNearestDistance_" << x << "_" << y << "_" << k;
    
    string tmp = po.str();
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
    
    bzero(buffer,256);

    n = read(sockfd,buffer,255);    
    if (n < 0)
	 cout << "ERROR reading from socket" << endl;
    
    //cout<<"buffer = "<<buffer<<endl;
    
    close(sockfd);
    /*
    BSONObjBuilder b;
    b << "center" << BSON_ARRAY( x << y ) << "radius" << radius;
    BSONObj center = b.obj();
    RangeExecutions++;

    auto_ptr<DBClientCursor> cursor = c.query(coll, QUERY( "loc" << BSON("$within" << BSON("$center" << center) ) ) );

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
*/
    if(measurekNNTime){
        endC = clock();
        totalCPUTime += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
        gettimeofday(&end, NULL);
        totalTime += util.print_time(start, end);
    }

    return atof(buffer);
  
}


void GPOs_D::clearNextNN(){

    while(!nextNNList->empty()) {
        delete nextNNList->back();
        nextNNList->pop_back();
    }
    delete nextNNList;

    nextNNList = new vector<res_point*>();
    computedNN = returnedNN = finalNextNN = 0;
    flagNextNN = true;
}

int GPOs_D::getkNNExecutions(){
    return kNNExecutions;
}

int GPOs_D::getLocationExecutions(){
    return LocationExecutions;
}

int GPOs_D::getNextNNExecutions(){
    return NextNNExecutions;
}

int GPOs_D::getRangeExecutions(){
    return RangeExecutions;
}

