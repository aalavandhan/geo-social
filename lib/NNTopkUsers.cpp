// #include "mongo/client/dbclient.h"
// using namespace mongo;

#include "headersMemory.h"

// #include "GPOs/MongoDbGrid/GPOsMongo.h"
// #include "SPOs/MongoDbMap/SPOsMongo.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// #include "GPOs/MemoryGrid/GPOs_D.h"
// #include "SPOs/MemoryMap/SPOs_D.h"

// #include "SPOs/MemoryMapWeighted/Graph_D.h"

#ifndef GLOBALS
#define GLOBALS
double MIN_X = 0;
double MAX_X = 0;
double MIN_Y = 0;
double MAX_Y = 0;
double DATASET_SIZE = 0;
double POI_SIZE = 0;
double DELTA_X = 0;
double DELTA_Y = 0;
int MAXSC = 0;                // maximum number of friends plus one
double MAXDIST = 0;          // maximum distance between two points
int MAXT = 0;
#endif

double print_time(struct timeval &start, struct timeval &end){
    double usec;

    usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
    return usec;
}


int main(int argc, char *argv[])
{
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout.precision(15);

    cout << "# arguments  = " << argc << endl;
    if (argc != 9){
	cout << "Usage: " << argv[0] << " query_file query_type k w Bpower EXTRA_FLOATING_POINT_VAR radius m" << endl;
	return -1;
    }

	char * q_f = argv[1];
	int k = atof(argv[3]);
    double w = atof(argv[4]);
	double Bpow = atof(argv[5]); // this is the data-dependent paramter, for example, if set to 4 then the radius of search is determined to contain k^4 users. (if k = 5) then total users in range ~625
	double w_EXTRA = atof(argv[6]);
    double radius = atof(argv[7])*360/EARTH_CIRCUMFERENCE;
    double m = atof(argv[8]);
    // int incrStep = atof(argv[8]);
    int incrStep = 2000;

	char* f = (char*) malloc(sizeof(char)*4);
    strcpy(f, "sum");
	//Instantiate SPOS and GPOS for Localized MongoDB
	// SPOsMongo* sposMongo = new SPOsMongo("localhost","NNTOPK.socialGraph");
    // GPOsMongo* gposMongo = new GPOsMongo("localhost","NNTOPK.checkins");


    int queryType = 0;
    cout<<"Query is "<<argv[2]<<endl;
    // NPRU returns the top-k users based on their spatial proximity to a given query location, their popularity, and their similarity to an input set of terms;
    //input : coordinate x,y of a POI(e.g. restaurant), integer k (output size), weight w (removed for simplicity), set of textual terms (can represent a POI's description (e.g. Mexican tequila bar))
    //output : top k most popular users (highest social degree) with highest simlarity to the query terms (cosine similarity to textual input), and nearest to query location
	if (strcmp(argv[2], "GSK_NPRU") == 0) {
		queryType = 1;
	}
	//FSKR discovers the top-k keywords based on their frequency in pairs of friends located within a spatial area.
 	//input : coordinate x,y location along with radius r specifies the spatial area, k determines the number of output words
    //output : top-k keywords based on their frequency in pairs of friends in the region around specified location
	else if (strcmp(argv[2], "GSK_FSKR") == 0) { //We can shoose to ignore this query for now since its so different from others
		queryType = 2;
	}
	// NSTP outputs the top-k POIs based on their proximity to a user v, the number of check-ins by friends of v, and their similarity to a set of terms;
    //input : coordinate x,y of a user location, integer k (output size), weight w (removed for simplicity), set of textual terms (can represent a user's preferences (e.g. Mexican tequila bar))
    //output : top k POIs with highest simlarity to the query terms (cosine similarity to textual input), and nearest to users location
	else if (strcmp(argv[2], "GSK_NSTP") == 0){
		queryType = 3;
	}
 	//NSG returns the k nearest groups of m users to a query location q, such that the users in every group are connected through a common friend. This query is based on the concept of aggregate summation of distance of users in the group
	//input : coordinate x,y of location, integer k (output size), integer w (group size), function f (hard coded to "sum" for simplicity (other options are "max" "min")), incrstep - a parameter to control incremental nearest neighbour query (hard coded to 2000 for simplictiy)
    //output : returns k nearest Star Groups (SG) (i.e. a user u u and m−1 of his/her friends)
 	else if (strcmp(argv[2], "NSG_LAZY") == 0){
		queryType = 4;
	}
	//------------All queries below belong to the following setting--------------
	//Given a query location q, Geo-Social Ranking (GSR) ranks the users of a Geo-Social Network based on their distance to q, the number of their friends in the vicinity of q, and possibly the connectivity of those friends.
	//---------------------------------------------------------------------------
	//LC, which is a weighted linear combination of social (i.e., friendships) and spatial (i.e., distance to q) aspects,
	//In LC, the score of a user vi is the weighted sum of the normalized social and spatial scores of vi. For the social score, we consider the number of relevant friends of vi, whereas for the spatial score we consider their distances to the query point.
	//subscripts UD and DD stand for user defined and data dependant. These are different techniques to specify the query range.
	//Ideally, the relevant range should contain a number of users K (K > k) large enough to include friends of the top-k users, but at the same time it should preserve
	//the query locality. To resolve this issue, we propose two approaches for setting r: i) user-defined and ii) data-dependent. In i), the user explicitly specifies the value of
	//r (e.g., a merchant may be interested in potential customers within 1km from his location). In ii), the value of r is set so that the expected number K of users within
	//the relevant range is a function of k (e.g., K = k4). For this estimation, we use a multi-dimensional histogram capturing the user locations and the cost model of
    // Y. Tao, J. Zhang, D. Papadias, and N. Mamoulis. Anefficient cost model for optimization of nearest neighbor search in low and medium dimensional spaces.Knowledge and Data Engineering, IEEE Transactions on,16(10):1169–1184, 2004.
	else if (strcmp(argv[2], "LCA_UD") == 0){
		queryType = 5;
	}
    else if (strcmp(argv[2], "LCA_DD") == 0){
		queryType = 6;
	}
	//RC is a ratio combination of the two aspects,
	//the RC score of a user v is proportional to the cardinality of the relevant set of his friends V, and inversely proportional to the sum of distances between q and the users in V
    else if (strcmp(argv[2], "RCA") == 0){
		queryType = 7;
	}
	//HGS, which considers the number of friends in coincident circles centered at q
	// this query is really hard to explain without going into the details, so take it at its face value.
	//UD and DD are different techniques to specify the range.
    else if (strcmp(argv[2], "HGS_DD") == 0){
		queryType = 8;
    }
    else if (strcmp(argv[2], "HGS_UD") == 0){
		queryType = 9;
	}
	// GST, which takes into account triangles of friends in the vicinity of q.
	// Geo-Social Triangles (GST) is motivated by the Geographic Clustering Coefficient, which combines the social clustering coefficient with spatial criteria, and the Average Triangle
	// Length metric, which is the average length of the triangles that a user forms with his friends.
	// EAGER runs several times faster but consumes more memory, LAZY is slower (around 1 sec per query)
    else if (strcmp(argv[2], "GST_EAGER") == 0){
		queryType = 10;
	}
	else if (strcmp(argv[2], "GST_LAZY") == 0){
		queryType = 11;
    }
    else{
		cout << "Unknown query type." << endl;
		return -1;
    }

		//Instantiate SPOS, GPOS & IVGEN for Main Memory		----------------------LAS VEGAS
	char* dataset_initials = (char*) malloc(sizeof(char)*3);
	strncpy(dataset_initials, q_f, 2);
	string dt_initials_s(dataset_initials);
	std::string dt_initials = dt_initials_s.substr(0,2);

	// HARDCODED TO LV
	dt_initials = "LV";


	const char *data_dir;
	SPOs* spos;
	GPOs* gpos;
	InvertedIndexGen* ivgen;


	//Instantiate SPOS, GPOS & IVGEN for Main Memory
	if(dt_initials == "LV"){

		MIN_X = -115.339468;
		MAX_X = -114.927464;
		MIN_Y = 35.996499;
		MAX_Y = 36.328529;
		MAXSC = 2451;
		MAXDIST = 0.53;
		POI_SIZE = 12773;
		DATASET_SIZE =  40298;
		DELTA_X = ((MAX_X - MIN_X)/ (double) (X-1));
		DELTA_Y = ((MAX_Y - MIN_Y)/ (double) (Y-1));

		data_dir = "dataLV/";
		spos = new SPOs();
		spos->load("dataLV/socialGraph.txt");
		if(queryType == 2){
			gpos = new GPOs("dataLV/checkins.txt", "dataLV/keywordsR.txt", spos);
		} else {
			gpos = new GPOs("dataLV/checkins.txt", "dataLV/keywords.txt", spos);
		}

		if(queryType == 1 || queryType == 2 || queryType == 3){
			gpos->loadPOIeverything("dataLV/locationPOI.txt", "dataLV/historyPOI.txt", "dataLV/keywordsPOI.txt", "dataLV/frequencyPOI.txt");
			gpos->createQuadTree();

			ivgen = new InvertedIndexGen();    	// Create the inverted index generator.
			// if(ivgen->loadFileWithTF("dataLV/keywordsWithTF.txt") == 0)
				// cout << "Keyword inverted index built in ivegen" <<endl;
			if(queryType == 2){
				if(ivgen->loadFileToIDX("dataLV/keywordsR.txt") == 0)
					cout << "Keyword inverted index built in ivgen" <<endl;
			} else {
				if(ivgen->loadFileToIDX("dataLV/keywords.txt") == 0)
					cout << "Keyword inverted index built in ivgen" <<endl;
			}

			if(ivgen->loadPOIToIDX("dataLV/keywordsPOI.txt") == 0)
				cout << "Keyword inverted index built in ivgen" <<endl;
		}
	}
	else if(dt_initials == "PX"){

		MIN_X = -112.495437;
		MAX_X = -111.711288;
		MIN_Y = 33.219214;
		MAX_Y = 33.859067;
		MAXSC = 1246;
		MAXDIST = 1.01;
		DATASET_SIZE =  30057;
		POI_SIZE = 16154;
		DELTA_X = ((MAX_X - MIN_X)/ (double) (X));
		DELTA_Y = ((MAX_Y - MIN_Y)/ (double) (Y));


		data_dir = "dataPX/";
		spos = new SPOs();
		spos->load("dataPX/socialGraph.txt");
		gpos = new GPOs("dataPX/checkins.txt", "dataPX/keywords.txt", spos);
		gpos->loadPOIeverything("dataPX/locationPOI.txt", "dataPX/historyPOI.txt", "dataPX/keywordsPOI.txt", "dataPX/frequencyPOI.txt");
		gpos->createQuadTree();

		ivgen = new InvertedIndexGen();    	// Create the inverted index generator.
		if(ivgen->loadFileToIDX("dataPX/keywords.txt") == 0)
			cout << "Keyword inverted index built in ivgen" <<endl;
		if(ivgen->loadPOIToIDX("dataPX/keywordsPOI.txt") == 0)
			cout << "Keyword inverted index built in ivgen" <<endl;
		// if(ivgen->loadFileWithTF("dataPX/keywordsWithTF.txt") == 0)
			// cout << "Keyword inverted index built in ivegen" <<endl;;
	}
	else
		return 0;


	SimpleQueries* sq = new SimpleQueries(gpos, spos);
    mGroup* mg = new mGroup(gpos,spos,sq);

    topkUsers* tku = new topkUsers(gpos, spos, sq);
	topkGSK* tkGSK = new topkGSK(gpos, spos, ivgen , sq);


    double x =0;
    double y =0;

    priority_queue <Group*, vector<Group*>, Group::ascending_score >* heap2;

    struct timeval start, end;


    double sum = 0, estimationSum = 0;
    double cumulativeAdist = 0.0;
    //double cumulativeDensity = 0;
    double cumulativeVsize =0;
    double cumulativeLongest_distance=0;
    double cumulativeUser_dist =0;
    double cumulativeThreshold=0;
    int cumulativeUsersChecked =0;

	double cumulativesposTimeSparse = 0;
	double cumulativegposTimeSparse = 0;
    double sumSparse = 0, estimationSumSparse= 0;
    double cumulativeAdistSparse = 0.0;
    //double cumulativeDensitySparse = 0;
    double cumulativeVsizeSparse =0;
    double cumulativeLongest_distanceSparse=0;
    double cumulativeUser_distSparse =0;
    double cumulativeThresholdSparse=0;
    int cumulativeUsersCheckedSparse =0;



	//Manually feed the number of query points in the dataset
	//to get summary of the query executions
	int queryPoints = 20; // 20;

	// string query_file = string(data_dir) + string(q_f);
	string query_file = string(q_f);
	ifstream infile(query_file.c_str());
    if (! infile){
		cout << "Cannot open query file " << argv[1] << "." << endl;
		return -1;
    }


    int times = 0;
	vector<int> amigos = {3336,19423,20756,7212};
    while(infile && times < queryPoints) {
	/////////////////////////////////
	// cout<< "--------------------------------------" << endl;


	vector<string>* terms = new vector<string>();
	string line="";
	if (getline(infile, line)) {
		char *bigBuffer = new char[line.size()+1];
		bigBuffer[line.size()]=0;
		memcpy(bigBuffer, line.c_str(),line.size());

		char *split;
		char *saveptr;
		int i = 0;
		split = strtok_r(bigBuffer," ",&saveptr);
		string word;
		cout <<"Query: ";
		while(split != NULL){
			cout<<split<<" ";
			if(strlen(split) > 0){
				if(i == 0){
					x = atof( split );
				}
				else if(i == 1){
					y = atof( split );
				}
				else{
					string temp(split);
					word = temp;
					terms->push_back(word);
				}
			}
			split = strtok_r(NULL, " ",&saveptr);
			i++;
		}
		cout<<"\n";
	}
	else{
		// cout << "Query point failed on line " << times << "." << endl;
		continue;

	}
	// cout<< "--------------------------------------" << endl;

	cout << "Query execution: " << times+1 << " -> Query location: (" << x << ", " << y << ")\n";


///

////////////////////////////////

	if (queryType == 1){ //GSK_NPRU
	    cout<<"Running GSK query with k = " << k << endl;
	    cout<<"and incremental steps = " << incrStep << " | weight = "<<w<<endl;

	    gettimeofday(&start, NULL);
		heap2 = tkGSK->NPRU(x,y,k,w,terms);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

		vector<int>* result_vec = new vector<int>();

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;// double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;

		cout <<"--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    //                   cout<<"heap2size"<<heap2->top()->size()<<endl;
		    heap2->top()->removeDuplicates();
		    //                    cout<<"heap2size"<<heap2->top()->size()<<endl;
			vector<int> friendList = heap2->top()->giveFriends();
			//spos->printTriangles(heap2->top()->id,friendList);
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    // density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

			result_vec->push_back(heap2->top()->getId());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    i--;
		}
		cout <<"--------------------------------------" << endl;
		delete heap2;
		}


		// cout<<"Number of getFriends calls = "<<spos->getGetFriendsExecutions() <<endl;
		// cout<<"Number of spos cache hits = "<<spos->getCacheHits() <<endl;
		// cout<<"SPOS Hit Percentage = "<< (double)  spos->getCacheHits() / spos->getGetFriendsExecutions() << endl;
		// spos->clearExecutionHistory();

		// cout<<"Number of getLocation calls = "<<gpos->getLocationExecutions() <<endl;
		// cout<<"Number of gpos cache hits = "<<gpos->getCacheHits() <<endl;
		// cout<<"GPOS Hit Percentage = "<< (double) gpos->getCacheHits() / gpos->getLocationExecutions()  << endl;
		// gpos->clearExecutionHistory();

	    cumulativeUser_dist+=user_dist/k;
	    cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tkGSK->getNumOfUsersChecked();
	    cout <<"Number of users checked = "<<tkGSK->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 2){ //FSKR

	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running FSKR query with k = " << k << endl;

	    cout<< "--------------------------------------" << endl;
	    gettimeofday(&start, NULL);
			heap2 = tkGSK->FSKR(x,y,k,w,radius);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<< "--------------------------------------" << endl;
	    cout<<"Single Query Time = "<<print_time(start,	 end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;// double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		// cout<<"Result set contains "<<i<<" users."<<endl;
		// cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    //                   cout<<"heap2size"<<heap2->top()->size()<<endl;
		    heap2->top()->removeDuplicates();
		    //                    cout<<"heap2size"<<heap2->top()->size()<<endl;
			vector<int> friendList = heap2->top()->giveFriends();
			//spos->printTriangles(heap2->top()->id,friendList);
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    // density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    // heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		// cout<< "--------------------------------------" << endl;
		delete heap2;
	    }

		// cout<<"Number of getFriends calls = "<<spos->getGetFriendsExecutions() <<endl;
		// cout<<"Number of spos cache hits = "<<spos->getCacheHits() <<endl;
		// cout<<"SPOS Hit Percentage = "<< (double)  spos->getCacheHits() / spos->getGetFriendsExecutions() << endl;
		// spos->clearExecutionHistory();

		// cout<<"Number of getLocation calls = "<<gpos->getLocationExecutions() <<endl;
		// cout<<"Number of gpos cache hits = "<<gpos->getCacheHits() <<endl;
		// cout<<"GPOS Hit Percentage = "<< (double) gpos->getCacheHits() / gpos->getLocationExecutions()  << endl;
		// gpos->clearExecutionHistory();

	    cumulativeUser_dist+=user_dist/k;
	    cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tkGSK->getNumOfUsersChecked();
	    cout <<"Number of users checked = "<<tkGSK->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 3){ //NSTP

	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running NSTP query with k = " << k << endl;
	    cout<< "--------------------------------------" << endl;
	    gettimeofday(&start, NULL);

		//for PX = 26748 LV = 32

		// int USER_ = amigos.at(times);;
		// int USER_ = 26748;
		int USER_ = 1265;

		// int USER_ = 3336;
		// int USER_ = 32;
		// cout<<"User is : "<<USER_<<"\n";
		heap2 = tkGSK->NSTP_VIS( x, y, k, w, USER_, terms);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<< "--------------------------------------" << endl;
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;// double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;

		// cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    //                   cout<<"heap2size"<<heap2->top()->size()<<endl;
		    // heap2->top()->removeDuplicates();
		    //                    cout<<"heap2size"<<heap2->top()->size()<<endl;
			// vector<int> friendList = heap2->top()->giveFriends();
			//spos->printTriangles(heap2->top()->id,friendList);
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    // density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    // heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		// cout<< "--------------------------------------" << endl;
		delete heap2;
	    }

		// cout<<"Number of getFriends calls = "<<spos->getGetFriendsExecutions() <<endl;
		// cout<<"Number of spos cache hits = "<<spos->getCacheHits() <<endl;
		// cout<<"SPOS Hit Percentage = "<< (double)  spos->getCacheHits() / spos->getGetFriendsExecutions() << endl;
		// spos->clearExecutionHistory();

		// cout<<"Number of getLocation calls = "<<gpos->getLocationExecutions() <<endl;
		// cout<<"Number of gpos cache hits = "<<gpos->getCacheHits() <<endl;
		// cout<<"GPOS Hit Percentage = "<< (double) gpos->getCacheHits() / gpos->getLocationExecutions()  << endl;
		// gpos->clearExecutionHistory();

	    cumulativeUser_dist+=user_dist/k;
	    cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tkGSK->getNumOfUsersChecked();
	    cout <<"Number of users checked = "<<tkGSK->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 4){ // NSG_LAZY

            // cout<< "--------------------------------------" << endl;
            cout<<"Running NSG LAZY with k = "<<k<<endl;
            cout<<"in m = "<<m<<endl;

            gettimeofday(&start, NULL);
            priority_queue <Group*, vector<Group*>, Group::descending >* heap2 = mg->LAZY(x, y, m, k,incrStep,f);;
            gettimeofday(&end, NULL);

            sum += print_time(start, end)/1000;  // compute ms
            cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

            double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;//double density=0;
            if(heap2 != NULL)
            {
                int i = heap2->size();
                cout<<"Result set contains "<<i<<" users."<<endl;\
                cout<< "--------------------------------------" << endl;
                while (!heap2->empty())
                {
                    aDist+=heap2->top()->adist;
                    Vsize+=heap2->top()->size();
                    user_dist+=heap2->top()->user_dist;
                    //density+=tku->getDensity(heap2->top());
                    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

                    // cout<<"User number: "<<i<<":"<<endl;
                    heap2->top()->print();
                    heap2->pop();
                    // cout <<"--------------------------------------" << endl;
                    i--;
                }
                cout<< "--------------------------------------" << endl;
                delete heap2;
            }

            cumulativeUser_dist+=user_dist/k;
            // cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
            cumulativeAdist+=aDist/k;
            // cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
            //cumulativeDensity+=density/k;
            //cout <<"Density averaged top-k = "<<density/k<<endl;
            //cumulativeThreshold+=tku->getThreshold();
            // cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
            // cumulativeUsersChecked+=tku->getNumOfUsersChecked();
            // cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
            cumulativeVsize+=Vsize/k;
            // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
            cumulativeLongest_distance+=longest_dist;
            // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
            // cout <<"--------------------------------------------------------------------------------------------------------" << endl;

	}
	else if (queryType == 5){ // LCA_UD
	    // cout<< "--------------------------------------" << endl;
	    cout<<"Running User defined LCA with k = "<<k<<endl;
	    cout<<"in a user defined radius = "<<radius<<" | "<<radius*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
	    // cout<< "--------------------------------------" << endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->LCA(x, y, k, w, radius);
	    gettimeofday(&end, NULL);

	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;//double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    //density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }
	    cumulativeUser_dist+=user_dist/k;
	    // cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    // cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    //cumulativeDensity+=density/k;
	    //cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    // cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    // cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 6){ //LCA_DD
	    int B = pow(k,Bpow);
	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running Data-dependent LC with k = "<<k<<" such that there are"<<endl;
	    cout<<"B = "<<B<<" users within the boundaries of the circle."<<endl;


	    gettimeofday(&start, NULL);
	    radius = gpos->estimateNearestDistance(x,y,B);
	    gettimeofday(&end, NULL);
	    estimationSum += print_time(start, end);


	    cout<<"Estimated radius = "<<radius<<" | "<<radius*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
	    // cout <<"--------------------------------------" << endl;
	    cout<<"Single Estimation Time = "<<print_time(start, end)<<" microsec"<<endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->LCA(x, y, k, w, radius);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0; //double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    //density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }
	    cumulativeUser_dist+=user_dist/k;
	    cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    // cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    // cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 7){ //RC

	    // cout <<"--------------------------------------" <<endl;
	    cout<<"Running RC with k = "<<k<<" | w = "<< w <<endl;
	    cout<<"and incremental steps = "<<incrStep<<endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->RC(x, y, k, incrStep, w);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; double Vsize =0;double longest_dist = 0; double user_dist = 0; //double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    //density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }


		//Applicable to main memory only
		// cout<<"Number of getFriends calls = "<<spos->getGetFriendsExecutions() <<endl;
		// cout<<"Number of spos cache hits = "<<spos->getCacheHits() <<endl;
		// cout<<"SPOS Hit Percentage = "<< (double)  spos->getCacheHits() / spos->getGetFriendsExecutions() << endl;
		// spos->clearExecutionHistory();

		// cout<<"Number of getLocation calls = "<<gpos->getLocationExecutions() <<endl;
		// cout<<"Number of gpos cache hits = "<<gpos->getCacheHits() <<endl;
		// cout<<"GPOS Hit Percentage = "<< (double)  gpos->getCacheHits() / gpos->getLocationExecutions() << endl;
		// gpos->clearExecutionHistory();


	    cumulativeUser_dist+=user_dist/k;
	    cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 8){ //HGS_DD

	    int B = pow(k,Bpow);
	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running Data-dependent HGS with k = "<<k<<" such that there are"<<endl;
	    cout<<"B = "<<B<<" users within the boundaries of the circle."<<endl;

	    gettimeofday(&start, NULL);
	    radius = gpos->estimateNearestDistance(x,y,B);
	    gettimeofday(&end, NULL);
	    estimationSum += print_time(start, end);

	    cout<<"Estimated radius = "<<radius<<" | "<<radius*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;
	    // cout <<"--------------------------------------" << endl;
	    cout<<"Single Estimation Time = "<<print_time(start, end)<<" microsec"<<endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->HGS(x, y, k, radius);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0;  int Vsize =0;double longest_dist = 0; double user_dist = 0; //double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    //density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }
	    cumulativeUser_dist+=user_dist/k;
	    // cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    // cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    // cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    // cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 9){ //HGS_UD

	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running User-defined h-GS index with d = "<<radius<<" | "<<radius*(EARTH_CIRCUMFERENCE/360)<<" km"<<endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->HGS(x, y, k, radius);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0; //double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    //density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }
	    cumulativeUser_dist+=user_dist/k;
	    // cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    // cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    // cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    // cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 10){ //GST_EAGER

	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running Geographic Clustering Algorithm with k = "<<k<<endl;
	    cout<<"and incremental steps = "<<incrStep<<" | beta = "<<w<<endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->GCA(x, y, k, incrStep,w);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0;int Vsize =0;double longest_dist = 0; double user_dist = 0;// double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    //                    cout<<"heap2size"<<heap2->top()->size()<<endl;
		    heap2->top()->removeDuplicates();
		    //                    cout<<"heap2size"<<heap2->top()->size()<<endl;
			vector<int> friendList = heap2->top()->giveFriends();
			//spos->printTriangles(heap2->top()->id,friendList);
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    //density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }
	    cumulativeUser_dist+=user_dist/k;
	    // cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    // cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    // cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    // cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    // cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    // cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else if (queryType == 11){ //GST LAZY

	    // cout <<"--------------------------------------" << endl;
	    cout<<"Running Geographic Clustering Algorithm with k = "<<k<<endl;
	    cout<<"and incremental steps = "<<incrStep<<" | weight = "<<w<<endl;

	    gettimeofday(&start, NULL);
	    heap2 = tku->GST_Lazy(x, y, k, incrStep,w);
	    gettimeofday(&end, NULL);
	    sum += print_time(start, end)/1000;  // compute ms
	    cout<<"Single Query Time = "<<print_time(start, end)/1000<<" ms"<<endl;

	    double aDist = 0; int Vsize =0;double longest_dist = 0; double user_dist = 0;// double density=0;
	    if(heap2 != NULL){
		int i = heap2->size();
		cout<<"Result set contains "<<i<<" users."<<endl;
		cout<< "--------------------------------------" << endl;
		while (!heap2->empty())
		{
		    //                   cout<<"heap2size"<<heap2->top()->size()<<endl;
		    heap2->top()->removeDuplicates();
		    //                    cout<<"heap2size"<<heap2->top()->size()<<endl;
			vector<int> friendList = heap2->top()->giveFriends();
			//spos->printTriangles(heap2->top()->id,friendList);
		    aDist+=heap2->top()->adist;
		    Vsize+=heap2->top()->size();
		    user_dist+=heap2->top()->user_dist;
		    // density+=tku->getDensity(heap2->top());
		    longest_dist= max(longest_dist,heap2->top()->getLongestDistance());

		    // cout<<"User number: "<<i<<":"<<endl;
		    heap2->top()->print();
		    heap2->pop();
		    // cout <<"--------------------------------------" << endl;
		    i--;
		}
		cout<< "--------------------------------------" << endl;
		delete heap2;
	    }

		// cout<<"Number of getFriends calls = "<<spos->getGetFriendsExecutions() <<endl;
		// cout<<"Number of spos cache hits = "<<spos->getCacheHits() <<endl;
		// cout<<"SPOS Hit Percentage = "<< (double)  spos->getCacheHits() / spos->getGetFriendsExecutions() << endl;
		// spos->clearExecutionHistory();

		// cout<<"Number of getLocation calls = "<<gpos->getLocationExecutions() <<endl;
		// cout<<"Number of gpos cache hits = "<<gpos->getCacheHits() <<endl;
		// cout<<"GPOS Hit Percentage = "<< (double) gpos->getCacheHits() / gpos->getLocationExecutions()  << endl;
		// gpos->clearExecutionHistory();

	    cumulativeUser_dist+=user_dist/k;
	    cout <<"User distance averaged top-k = ("<<user_dist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeAdist+=aDist/k;
	    cout <<"Adist averaged top-k = ("<<aDist/k*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cumulativeDensity+=density/k;
	    // cout <<"Density averaged top-k = "<<density/k<<endl;
	    cumulativeThreshold+=tku->getThreshold();
	    cout <<"Threshold distance = ("<<tku->getThreshold()*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    cumulativeUsersChecked+=tku->getNumOfUsersChecked();
	    cout <<"Number of users checked = "<<tku->getNumOfUsersChecked()<<endl;
	    cumulativeVsize+=Vsize/k;
	    cout <<"Size of Vi averaged top-k = "<<((double)Vsize/k)<<endl;
	    cumulativeLongest_distance+=longest_dist;
	    cout <<"Longest distance among top-k = ("<<longest_dist*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
	    // cout <<"--------------------------------------------------------------------------------------------------------" << endl;
	}
	else{
		cout << "Query Type unrecognized for processing" <<endl;

	}

	if(times == ((queryPoints/2) - 1))   {
	    sumSparse = sum;
	    estimationSumSparse= estimationSum;
	    cumulativeAdistSparse = cumulativeAdist;
	    // cumulativeDensitySparse = cumulativeDensity;
	    cumulativeVsizeSparse =cumulativeVsize;
	    cumulativeLongest_distanceSparse=cumulativeLongest_distance;
	    cumulativeUser_distSparse =cumulativeUser_dist;
	    cumulativeThresholdSparse=cumulativeThreshold;
	    cumulativeUsersCheckedSparse =cumulativeUsersChecked;
		cumulativesposTimeSparse = ((double)spos->getTotalTime())/1000;
		cumulativegposTimeSparse = ((double)gpos->getTotalTime())/1000;

	    sum = 0;
	    estimationSum = 0;
	    cumulativeAdist = 0.0;
	    // cumulativeDensity = 0;
	    cumulativeVsize =0;
	    cumulativeLongest_distance=0;
	    cumulativeUser_dist =0;
	    cumulativeThreshold=0;
	    cumulativeUsersChecked =0;
	}

	terms->clear();
	times++;
    }

    cout << "Query Type = " << argv[2] << endl;

	//END execution if query file contains less than 2 query points
    if(times<3){
	delete sq;
	delete tku;
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
    //    cout << "Avg SPOs Total Time = " << ((double)spos->getTotalTime()/times) << "microseconds" << endl;
    //    cout << "Avg GPOs CPU Time = " << ((double)gpos->getTotalCPUTime()/times) << "ms" << endl;
    //    cout << "Avg GPOs Total Time = " << ((double)gpos->getTotalTime()/times) << "microseconds" << endl;

    int halfNumberOfQueries = times/2;
	cout<<"halfnumqueries = "<<halfNumberOfQueries<<endl;
	radius = radius*EARTH_CIRCUMFERENCE/360;
    cout<<"*************************************************************************"<<endl;
    cout<<"************************* SPARSE POINTS *********************************"<<endl;
    cout<<"*************************************************************************"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_userDist"<<"_sparse_("<<cumulativeUser_distSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_aDist"<<"_sparse_("<<cumulativeAdistSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    // cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_density"<<"_sparse_("<<cumulativeDensitySparse/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_threshold"<<"_sparse_("<<cumulativeThresholdSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_numOfUsersChecked"<<"_sparse_("<<cumulativeUsersCheckedSparse/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_Vsize"<<"_sparse_("<<cumulativeVsizeSparse/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_longestDist"<<"_sparse_("<<cumulativeLongest_distanceSparse/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgTime"<<"_sparse_("<< (sumSparse/halfNumberOfQueries) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_totalTime"<<"_sparse_("<<sumSparse << ") ms"<<endl;

	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgSPOSTime"<<"_sparse_("<<cumulativesposTimeSparse/halfNumberOfQueries << ") ms"<<endl;
	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgGPOSTime"<<"_sparse_("<<cumulativegposTimeSparse/halfNumberOfQueries<< ") ms"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgEstimTime"<<"_sparse_("<<(estimationSumSparse/halfNumberOfQueries)<<") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_totalEstimTime"<<"_sparse_("<< (estimationSumSparse)<< ") microseconds"<<endl;


    cout<<"*************************************************************************"<<endl;
    cout<<"************************* DENSE POINTS **********************************"<<endl;
    cout<<"*************************************************************************"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_userDist"<<"_dense_("<<cumulativeUser_dist/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_aDist"<<"_dense_("<<cumulativeAdist/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    // cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_density"<<"_dense_("<<cumulativeDensity/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_threshold"<<"_dense_("<<cumulativeThreshold/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_numOfUsersChecked"<<"_dense_("<<cumulativeUsersChecked/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_Vsize"<<"_dense_("<<cumulativeVsize/halfNumberOfQueries<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_longestDist"<<"_dense_("<<cumulativeLongest_distance/halfNumberOfQueries*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgTime"<<"_dense_("<< (sum/halfNumberOfQueries) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_totalTime"<<"_dense_("<<sum << ") ms"<<endl;

	cumulativesposTimeSparse = (((double)spos->getTotalTime())/1000) - cumulativesposTimeSparse;
	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgSPOSTime"<<"_dense_("<<cumulativesposTimeSparse/halfNumberOfQueries << ") ms"<<endl;

	cumulativegposTimeSparse = (((double)gpos->getTotalTime())/1000) - cumulativegposTimeSparse;
	cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgGPOSTime"<<"_dense_("<<cumulativegposTimeSparse/halfNumberOfQueries<< ") ms"<<endl;

    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgEstimTime"<<"_dense_("<<(estimationSum/halfNumberOfQueries)<<") microseconds"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_totalEstimTime"<<"_dense_("<< (estimationSum)<< ") microseconds"<<endl;

    cout<<"*************************************************************************"<<endl;
    cout<<"************************* TOTAL AGGREGATE *******************************"<<endl;
    cout<<"*************************************************************************"<<endl;


    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_userDist"<<"_all_("<<(cumulativeUser_dist+cumulativeVsizeSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_aDist"<<"_all_("<<(cumulativeAdist+cumulativeAdistSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    // cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_density"<<"_all_("<<(cumulativeDensity+cumulativeDensitySparse)/times<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_threshold"<<"_all_("<<(cumulativeThreshold+cumulativeThresholdSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_numOfUsersChecked"<<"_all_("<<(cumulativeUsersChecked+cumulativeUsersCheckedSparse)/times<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_Vsize"<<"_all_("<<(cumulativeVsize+cumulativeVsizeSparse)/times<<")"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_longestDist"<<"_all_("<<(cumulativeLongest_distance+cumulativeLongest_distanceSparse)/times*(EARTH_CIRCUMFERENCE/360)<<") km"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgTime"<<"_all_("<< ((sum+sumSparse)/times) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_totalTime"<<"_all_("<<(sum+sumSparse) << ") ms"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_avgEstimTime"<<"_all_("<< ((estimationSum+estimationSumSparse)/times) <<") microseconds"<<endl;
    cout <<k<<"_"<<w<<"_"<<radius<<"_"<<argv[2]<<"_totalEstimTime"<<"_all_("<< (estimationSum+estimationSumSparse) << ") microseconds"<<endl;

    //cout <<k<<"_"<<argv[2]<<"_totalPOTime"<<"_all_("<< ((spos->getTotalCPUTime()+gpos->getTotalCPUTime())/times) << ") ms" << endl;
    //cout <<k<<"_"<<argv[2]<<"_totalALGTime"<<"_all_("<< (((sum+sumSparse)-(spos->getTotalCPUTime()+gpos->getTotalCPUTime()))/times) << ") ms" << endl;

    delete sq;
    delete tku;
    return 0;
}


