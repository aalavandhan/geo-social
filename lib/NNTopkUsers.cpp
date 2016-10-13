#include "headersMemory.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include "GPOs/MemoryGrid/GPOs_D.h"
#include "SPOs/MemoryMap/SPOs_D.h"

#include "SPOs/MemoryMapWeighted/Graph_D.h"

#ifndef GLOBALS
#define GLOBALS
double MIN_X = 0;
double MAX_X = 0;
double MIN_Y = 0;
double MAX_Y = 0;
double DATASET_SIZE = 0;
double DELTA_X = 0;
double DELTA_Y = 0;
int MAXSC = 0;                // maximum number of friends plus one
double MAXDIST = 0;          // maximum distance between to points
int MAXT = 0;
#endif

double print_time(struct timeval &start, struct timeval &end){
    double usec;

    usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    return usec;
}


int main(int argc, char *argv[])
{
    // cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout.precision(15);

    clock_t startC, endC;

    // cout << "# arguments  = " << argc << endl;

    if (argc != 18){
	// cout << "Usage: " << argv[0] << " query_file grid_file query_type[LCA_UD | LCA_DD | UCA | HGS | TEST] k incrStep socialGraph {sum | max} w radius Num_of_files(for synthetic) minX maxX minY maxY MAXS MAXD MAXT" << endl;
	return -1;
    }


    ifstream fin(argv[1]);
    if (! fin){
	// cout << "Cannot open query file " << argv[1] << "." << endl;
	return -1;
    }


    double Bpow = atof(argv[10]); // number of files
    double radius = atof(argv[9])*360/EARTH_CIRCUMFERENCE;
    int k = atof(argv[4]);
    int incrStep = atof(argv[5]);
    char* f = (char*) malloc(sizeof(char)*3);
    strcpy(f, argv[7]);
    double w = atof(argv[8]);


	MIN_X = atof(argv[11]);
	MAX_X = atof(argv[12]);
	MIN_Y = atof(argv[13]);
	MAX_Y = atof(argv[14]);
	MAXSC = atoi(argv[15]);
	MAXDIST = atof(argv[16]);
	MAXT = atoi(argv[17]);


	DATASET_SIZE = 100000 * (int) Bpow;
	DELTA_X = ((MAX_X - MIN_X)/ (X-1));
	DELTA_Y = ((MAX_Y - MIN_Y)/ (Y-1));


	// cout<<"MIN_X: "<<MIN_X<<" MAX_X:"<<MAX_X<<" MIN_Y:"<<MIN_Y<<" MAX_Y:"<<MAX_Y<<endl;
	// cout<<"DELTA_X: "<<DELTA_X<<" DELTA_Y: "<<DELTA_Y<<" Files Read: "<<Bpow<<" DATASET_SIZE: "<<DATASET_SIZE;
	// cout<<"============================Query Parameters for query "<<argv[3]<<" ================================"<<"\n";
	// cout<<"Radius: "<<argv[9]<<" km "<<" Weight: "<<w<<" k: "<<k<<" incrStep: "<< incrStep<<"\n";
	// cout<<"Query File: "<<argv[1]<< "\n";
	// cout<<"Checkins File: "<<argv[2]<<"\n";
	// cout<<"SocialGraph File: "<<argv[6]<<"\n";
	// cout<<"====================================================================================================="<<"\n";

    SPOs* spos = new SPOs();
    spos->load(argv[6],(int)Bpow);
    GPOs* gpos = new GPOs(argv[2], (int)Bpow);

	//char host[] = "143.89.197.187"; //Nikos Desktop
	//char host[] = "143.89.191.111" //stcpu1
	//char host[] = "143.89.191.112"; //stcpu2
	//SPOs_D *spos = new SPOs_D(8080, host);
	//GPOs_D *gpos = new GPOs_D(8080, host);



//    char host[] = "143.89.197.191";

//    char host[] = "localhost";
//    SPOs_D *spos_D = new SPOs_D(8080, host);
//    GPOs_D *gpos_D = new GPOs_D(8080, host);


    //int V = 12748;
    //Graph* g = new Graph(V);
    //double totalSocialWeights = g->load("data/socialGraph_w1.txt");
    //Graph_D *g_D = new Graph_D(8080, host);





    SimpleQueries* sq = new SimpleQueries(gpos, spos);

    int queryType = 0;

    if (strcmp(argv[3], "GF1") == 0)
	queryType = 1;
    else{
	// cout << "Unknown query type." << endl;
	return -1;
    }
    //    double x = atof(argv[9]);
    //    double y = atof(argv[10]);

    double x =0;
    double y =0;


    struct timeval start, end;

    double sum = 0, estimationSum = 0;
    double cumulativeAdist = 0.0;
    //double cumulativeDensity = 0;
    double cumulativeVsize =0;
    double cumulativeLongest_distance=0;
    double cumulativeUser_dist =0;

	double cumulativesposTimeSparse = 0;
	double cumulativegposTimeSparse = 0;
    double sumSparse = 0, estimationSumSparse= 0;
    double cumulativeAdistSparse = 0.0;
    //double cumulativeDensitySparse = 0;
    double cumulativeVsizeSparse =0;
    double cumulativeLongest_distanceSparse=0;
    double cumulativeUser_distSparse =0;


    //Cell* c = gpos->grid->getCell(x, y);
    //cout << "query cell = (" << c->getIndexI() << ", " << c->getIndexJ() << ")\n";

    //cout << "query location = (" << c->getIndexI() << ", " << c->getIndexJ() << ")\n";

    int times = 0;
    while(fin && times < 40) {


	fin >> x >> y;
	if (! fin.good()){
	    // cout<< "query point failed!";
	    continue;
	}

	if( x < MIN_X || x > MAX_X || y < MIN_Y || y > MAX_Y){
		// cout << "query location = (" << x << ", " << y << ")\n";
		// cout<< "Query point is out of bounds \n";
		continue;
	}

	// cout<< "--------------------------------------" << endl;
	// cout << "Query execution: " << times+1 << endl;
	// cout << "query location = (" << x << ", " << y << ")\n";
	// cout<< "--------------------------------------" << endl;




	// FOR TESTING I AM USING w as m



	if (queryType == 1){ // Get Friends In Range 1

		// FOR TESTING I AM USING user with id 10
		auto it = gpos->locations.find(2);
		double p_X;
		double p_Y;
		if(it!=gpos->locations.end()){
			p_X= (*it).second->getX();
			p_Y= (*it).second->getY();
		}
		else{
			p_X = p_Y = -1000;
		}

		Utilities util;
		res_point* newP = util.createResultPoint(2, p_X, p_Y, 0);


	 //    cout<< "--------------------------------------" << endl;
	 //    cout<<"Running Get Friends 1 query for  user = "<< newP->id <<endl;
		// cout<<"in a user defined radius = "<<radius<<" | "<<radius*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
		// cout<< "--------------------------------------" << endl;

	    gettimeofday(&start, NULL);
	    startC = clock();
	    Group* newG = sq->getMyFriendsInRange1(x, y, radius, newP);
	    endC = clock();
	    gettimeofday(&end, NULL);

	    sum += print_time(start, end)/1000;  // compute ms
	    // cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;//double density=0;

		aDist+=newG->adist;
		Vsize+=newG->size();
		user_dist+=newG->user_dist;

		longest_dist= max(longest_dist,newG->getLongestDistance());

		// cout<<"Printing Group:"<<endl;
		newG->print();


	    cumulativeUser_dist+=user_dist/k;
	    // cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    // cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    //cumulativeDensity+=density/k;
	    //cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeVsize+=Vsize/k;
	    // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else{
	    // cout << "Unknown query type." << endl;
	    return -1;
	}

	if(times==19)   {
	    sumSparse = sum;
	    estimationSumSparse= estimationSum;
	    cumulativeAdistSparse = cumulativeAdist;
	    cumulativeVsizeSparse =cumulativeVsize;
	    cumulativeLongest_distanceSparse=cumulativeLongest_distance;
	    cumulativeUser_distSparse =cumulativeUser_dist;
		cumulativesposTimeSparse = ((double)spos->getTotalTime())/1000;
		cumulativegposTimeSparse = ((double)gpos->getTotalTime())/1000;

	    sum = 0;
	    estimationSum = 0;
	    cumulativeAdist = 0.0;
	    cumulativeVsize =0;
	    cumulativeLongest_distance=0;
	    cumulativeUser_dist =0;
	}

	times++;
    }

    // cout << "Query Type = " << argv[3] << endl;

    if(times<3){
	delete sq;
	return 0;
    }

    //    cout << "Avg SPOs page accesses = " << ((double)spos->getPageAccesses()/times) << endl;
    //    cout << "Avg GPOs page accesses = " << ((double)gpos->getPageAccesses()/times) << endl;
    //    cout << "Avg kNNExecutions = " << ((double)gpos->getkNNExecutions()/times) << endl;
    //    cout << "Avg LocationExecutions = " << ((double)gpos->getLocationExecutions()/times) << endl;
    //    cout << "Avg NextNNExecutions = " << ((double)gpos->getNextNNExecutions()/times) << endl;
    //    cout << "Avg RangeExecutions = " << ((double)gpos->getRangeExecutions()/times) << endl;
    //    cout << "Avg areFriendsExecutions = " << ((double)spos->getAreFriendsExecutions()/times) << endl;
    //    cout << "Avg getFriendsExecutions = " << ((double)spos->getGetFriendsExecutions()/times) << endl;
    //    cout << "Avg SPOs CPU Time = " << ((double)spos->getTotalCPUTime()/times) << "ms" << endl;
    //    cout << "Avg SPOs Total Time = " << ((double)spos->getTotalTime()/times)/1000 << "ms" << endl;
    //    cout << "Avg GPOs CPU Time = " << ((double)gpos->getTotalCPUTime()/times) << "ms" << endl;
    //    cout << "Avg GPOs Total Time = " << ((double)gpos->getTotalTime()/times)/1000 << "ms" << endl;


    int halfNumberOfQueries = times/2;
	cout<<"halfnumqueries = "<<halfNumberOfQueries<<endl;
	radius = radius*EARTH_CIRCUMFERENCE/360;
    cout<<"*************************************************************************"<<endl;
    cout<<"************************* SPARSE POINTS *********************************"<<endl;
    cout<<"*************************************************************************"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_userDist"<<"_sparse_("<<cumulativeUser_distSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_aDist"<<"_sparse_("<<cumulativeAdistSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    // cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_density"<<"_sparse_("<<cumulativeDensitySparse/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_Vsize"<<"_sparse_("<<cumulativeVsizeSparse/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_longestDist"<<"_sparse_("<<cumulativeLongest_distanceSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgTime"<<"_sparse_("<< (sumSparse/halfNumberOfQueries) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_totalTime"<<"_sparse_("<<sumSparse << ") ms"<<endl;

	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgSPOSTime"<<"_sparse_("<<cumulativesposTimeSparse/halfNumberOfQueries << ") ms"<<endl;
	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgGPOSTime"<<"_sparse_("<<cumulativegposTimeSparse/halfNumberOfQueries<< ") ms"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgEstimTime"<<"_sparse_("<<(estimationSumSparse/halfNumberOfQueries)<<") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_totalEstimTime"<<"_sparse_("<< (estimationSumSparse)<< ") microseconds"<<endl;


    cout<<"*************************************************************************"<<endl;
    cout<<"************************* DENSE POINTS **********************************"<<endl;
    cout<<"*************************************************************************"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_userDist"<<"_dense_("<<cumulativeUser_dist/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_aDist"<<"_dense_("<<cumulativeAdist/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    // cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_density"<<"_dense_("<<cumulativeDensity/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_Vsize"<<"_dense_("<<cumulativeVsize/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_longestDist"<<"_dense_("<<cumulativeLongest_distance/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgTime"<<"_dense_("<< (sum/halfNumberOfQueries) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_totalTime"<<"_dense_("<<sum << ") ms"<<endl;

	cumulativesposTimeSparse = (((double)spos->getTotalTime())/1000) - cumulativesposTimeSparse;
	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgSPOSTime"<<"_dense_("<<cumulativesposTimeSparse/halfNumberOfQueries << ") ms"<<endl;

	cumulativegposTimeSparse = (((double)gpos->getTotalTime())/1000) - cumulativegposTimeSparse;
	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgGPOSTime"<<"_dense_("<<cumulativegposTimeSparse/halfNumberOfQueries<< ") ms"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgEstimTime"<<"_dense_("<<(estimationSum/halfNumberOfQueries)<<") microseconds"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_totalEstimTime"<<"_dense_("<< (estimationSum)<< ") microseconds"<<endl;

    cout<<"*************************************************************************"<<endl;
    cout<<"************************* TOTAL AGGREGATE *******************************"<<endl;
    cout<<"*************************************************************************"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_userDist"<<"_all_("<<(cumulativeUser_dist+cumulativeVsizeSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_aDist"<<"_all_("<<(cumulativeAdist+cumulativeAdistSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    // cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_density"<<"_all_("<<(cumulativeDensity+cumulativeDensitySparse)/times<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_Vsize"<<"_all_("<<(cumulativeVsize+cumulativeVsizeSparse)/times<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_longestDist"<<"_all_("<<(cumulativeLongest_distance+cumulativeLongest_distanceSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgTime"<<"_all_("<< ((sum+sumSparse)/times) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_totalTime"<<"_all_("<<(sum+sumSparse) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_avgEstimTime"<<"_all_("<< ((estimationSum+estimationSumSparse)/times) <<") microseconds"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[3]<<"_totalEstimTime"<<"_all_("<< (estimationSum+estimationSumSparse) << ") microseconds"<<endl;

    //cout <<k<<"_"<<argv[3]<<"_totalPOTime"<<"_all_("<< ((spos->getTotalCPUTime()+gpos->getTotalCPUTime())/times) << ") ms" << endl;
    //cout <<k<<"_"<<argv[3]<<"_totalALGTime"<<"_all_("<< (((sum+sumSparse)-(spos->getTotalCPUTime()+gpos->getTotalCPUTime()))/times) << ") ms" << endl;

    delete sq;
    return 0;
}


