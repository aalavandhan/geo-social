/*===================================================== Experiments ========================================================

Dataset: Synthetic, 1M points uniformly distributed 
Queries: Synthetic, 1M queries (points), in case of range query radius is standard 

Experiments: Same experiments for different grid size (num of cells)

- get_kNN Queries: k = 1, 10, 100, 1000, 10000

- get_nextNN: for a specific amount of  query points (10000) execute it 10000 times and get the average time (it is just a number)

- get_Range (radius in meters/users in range): r = 0.001(100m)/~22 , 0.005 (500m)/~635 , 0.01(1km)/~2596, 0.015(1.5km)/~5853, 0.02(2km)/~10332
	+ Here is better to check both approaches: (i) intersection and (ii) fully contains.   

- get_UserLocation: ask the location of each user and get the average time (it is just a number)

*/
#include <cstring>
#include <sys/time.h>
#include <time.h>

#include "headers.h"


// counts micro seconds
double print_time(struct timeval &start, struct timeval &end){
	double usec;
	
	usec = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
	return usec;
}


int main(int argc, char *argv[])
{

	clock_t startC, endC;	

	cout << "Start" << endl;

	if (argc != 7){
		cout << "Usage: " << argv[0] << " query_file grid_file query_type[get_kNN | get_Range | get_Range2 | get_nextNN | get_UserLocation] k range incrStep." << endl;
		return -1;
	}

	struct timeval start, end;

	ifstream fin(argv[1]);	
	if (! fin){
		cout << "Cannot open query file " << argv[1] << "." << endl;
		return -1;
	}

	ifstream rtree(argv[6]);	
	if (! fin){
		cout << "Cannot open query file " << argv[1] << "." << endl;
		return -1;
	}

	int queryType = 0;
	if (strcmp(argv[3], "get_kNN") == 0) 
		queryType = 0;
	else if (strcmp(argv[3], "get_Range") == 0) 
		queryType = 1;
	else if (strcmp(argv[3], "get_nextNN") == 0) 
		queryType = 2;
	else if (strcmp(argv[3], "get_UserLocation") == 0) 
		queryType = 3;
	else if (strcmp(argv[3], "get_Range2") == 0) 
		queryType = 4;
	else{
		cout << "Unknown query type." << endl;
		return -1;
	}
	
	int k = 0;
	if (strcmp(argv[4], "1") == 0) 
		k = 1;
	else if (strcmp(argv[4], "10") == 0) 
		k = 10;
	else if (strcmp(argv[4], "100") == 0) 
		k = 100;
	else if (strcmp(argv[4], "1000") == 0) 
		k = 1000;
	else if (strcmp(argv[4], "10000") == 0) 
		k = 10000;
	else{
		cout << "Unknown k." << endl;
		return -1;
	}	
	
	double range = atof(argv[5]);

	int incrStep = atof(argv[6]);

	cout << "Loading Grid ... " << endl; 
	Grid* grid = new Grid;
	grid->loadFromFile(argv[2]);
	cout << "Grid is ok!" << endl; 

	int times = 0;
	int getNext_times = 0;
	int id;
	int op;
	double x1, x2, y1, y2;
	double kNN_sum = 0;
	double nextNN_sum = 0;
	double range_sum = 0;
	double userLocation_sum = 0;

	int nextNN_executions = 10;

	int rtree_id, rtree_sum;

	int usersInRange = 0;


	if(queryType == 3){
	/*================================================ get_UserLocation Queries ==================================================*/
		Visitor* vis;
		
		cout << "Get_UserLocation" << endl;
		for(int i = 0; i < 1000000; i++){
			vis = new Visitor();			

			//gettimeofday(&start, NULL);
			startC = clock();			
			grid->getLocation(i, *(vis));
			endC = clock();			
			//gettimeofday(&end, NULL);
	  		//userLocation_sum += print_time(start, end);			
			userLocation_sum += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
			delete vis;

		}

		cout << "getUserLocation Experiments" << endl;
		cout << "Num of Queries = " << 1000000 <<" Avg time = " << (userLocation_sum/1000000) << " micro sec" << endl;
	}
	else{

		int visits = 0;
		int computations = 0; 

		//while (fin){
		while (fin && times < 100000){
		//while (fin && rtree && times < 1000){
			fin >> op >> id >> x1 >> y1 >> x2 >> y2;

			//rtree >> rtree_id >> rtree_sum;
			//rtree >> rtree_id;

			if (! fin.good()) continue; // skip newlines, etc.

			//if (! rtree.good()) continue; // skip newlines, etc.

			/*==================================================== get_kNN Queries ==================================================*/
			if(queryType == 0){

				Visitor* vis = new Visitor();
				vis->setX(x1);	
				vis->setY(y1);
				gettimeofday(&start, NULL);
				//startC = clock();	
				grid->getkNN(*(vis), k);
				//endC = clock();
				gettimeofday(&end, NULL);
	  			kNN_sum += print_time(start, end);			
				//kNN_sum += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
				delete vis;

			}
			/*=================================================== get_Range Queries ==================================================*/
			else if(queryType == 1){
			
			//	if(times == 108){

										

					Visitor* vis = new Visitor;
					//gettimeofday(&start, NULL);
					startC = clock();
					grid->getRange(x1, y1, range, *(vis));
					endC = clock();
					//gettimeofday(&end, NULL);
		  			//range_sum += print_time(start, end);
					range_sum += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
					usersInRange+=vis->getResult()->size();
				
					//visits += vis->getVisits();
					//computations += vis->getComputations(); 
					
					//cout << "Times = " << times << endl;

			/*	
					//if( vis->getResult()->size() != rtree_sum)	
					//	cout << "!!!!!!! Error!!!  " << times << " Grid = " << vis->getResult()->size() << " Rtree = " << rtree_sum << " " << x1 << ", " << y1 << endl;

					set<int> check;
					set<int> myset;
					set<int>::iterator it;
					while (rtree){
						rtree >> rtree_id;
						if (! rtree.good()) continue; // skip newlines, etc.
						myset.insert(rtree_id);
					}
					int sim = 0;
					cout << "Rtree count = " << myset.size() << "set[0] = " << (*myset.begin()) << endl;
					cout << "Grid count = " << vis->getResult()->size() << endl;
					while(!vis->getResult()->empty()){
						int id = vis->getResult()->back()->id;
						it=myset.find(id);
						check.insert(id);
						if(it == myset.end()){
							cout << id << " " << vis->getResult()->back()->x << ", " << vis->getResult()->back()->y << endl;
						}
						else{
							sim++;
							
						}
						vis->getResult()->pop_back();	
					}					
					cout << "Similar = " << sim << endl;
					cout << "Grid count (set) = " << check.size() << endl;
				*/
					delete vis;
				//}
			
			}
			/*=================================================== get_nextNN Queries ==================================================*/
			else if(queryType == 2){
				IncrVisitor* incrVis = new IncrVisitor;
				incrVis->setX(x1);
				incrVis->setY(y1);

				//gettimeofday(&start, NULL);
				startC = clock();
				for(int i=0; i< nextNN_executions; i++){
					grid->getNextNN(*(incrVis), incrStep);
				}
				//gettimeofday(&end, NULL);
				endC = clock();	  			
				//nextNN_sum += print_time(start, end);
				nextNN_sum += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
				getNext_times+=nextNN_executions;
				delete incrVis;
			}
			/*=================================================== get_Range2 Queries ==================================================*/
			else if(queryType == 4){
			
				Visitor* vis = new Visitor;
				//gettimeofday(&start, NULL);
				startC = clock();
				grid->getRange2(x1, y1, range, *(vis));
				endC = clock();
				//gettimeofday(&end, NULL);
		  		//range_sum += print_time(start, end);
				range_sum += (((double)(endC-startC)*1000.0)/(CLOCKS_PER_SEC));
				usersInRange+=vis->getResult()->size();
				
				delete vis;
			
			}
			else{	
				cout << "Unknown query type." << endl;
				return -1;
			}
			times++;

			if(times % 10000 == 0)
				cerr << "Query execution: " << times << endl;

		}


		cout << "Grid size = [" << X << ", " << Y << "]" << endl;
		cout << "Num of Points = 1M" << endl;

		if(queryType == 0){
			cout << "kNN Query Experiments" << endl; 

			cout << "k = " << k << endl; 
			cout << "Num of Queries = " << times <<" Avg time = " << (kNN_sum/times) << " ms" << endl;	

		}
		else if(queryType == 1){
			cout << "range Query Experiments" << endl; 

			cout << "range = " << range << endl;
			cout << "Num of users in range = " << (usersInRange/times) << endl; 
			cout << "Num of Queries = " << times <<" Avg time = " << (range_sum/times) << " ms" << endl;	
			cout << "Num of cells visited in avg = " << (double)((double)visits/times) <<" Num of mindist computations in avg = " << (computations/times) << endl;	

		}
		else if(queryType == 2){
			cout << "getnextNN Query Experiments" << endl; 

			cout << "IncrStep = " << incrStep << endl; 
			cout << "Executions per user = " << nextNN_executions << endl;
			cout << "Num of Queries = " << times <<" Avg time = " << (nextNN_sum/getNext_times) << " ms" << endl;	

		}
		else if(queryType == 4){
			cout << "range Query2 Experiments" << endl; 

			cout << "range = " << range << endl;
			cout << "Num of users in range = " << (usersInRange/times) << endl; 
			cout << "Num of Queries = " << times <<" Avg time = " << (range_sum/times) << " ms" << endl;	
			cout << "Num of cells visited in avg = " << (double)((double)visits/times) <<" Num of mindist computations in avg = " << (computations/times) << endl;	

		}
		else{
			cout << "error: There is no such type of query" << endl; 

		}

	}



	cout << "End" << endl;

	delete grid;

	return 0;
}


// --------------------------------------- TODO -------------------------------------------------//

// Done !!!!! 1. Check when you are going out of the table!
// Done !!!!! 2. Read the paper again, isNNAlready is not efficient.
// Done !!!!! 3. Clear the code!
// Done !!!!! 4. Implement kNN algorithm.
// Done !!!!! 5. Implement range query.
// Done !!!!! 6. Implement the deconstructors. Not for visitors
// Done !!!!! 7. Create objects only with new.
// Done !!!!! 8. Implement efficiently the main functions for distances, see Mario's code. 
// Done !!!!! 9. Call by reference, similar to marios' code.
// Done !!!!! 10. Check deletes in NN algorithms, when you create something, you should also delete it. we do not delete some cells in NN algorithms 
// 11. I must be careful with compute distance, if for example i have computed the min dist in an NN and then i have a nested range i want to have the proper dists!

// 12. Range query can be implemented more efficiently, check if the circle fully covers the cell.
// 13. Decide which functions should be public or not.



// In R-Tree
// 1. Check Marios' implementation about the result set, maybe it can be pointers to results. like INNEntry
// 2. deletions must hold.
// 3. check the knearest dist if we need it.



























/*



	cout << X << endl;

    clock_t t1, t2;
      
    cout << "Loading Grid ..." << endl;
    //loadSpatialDataset(&grid, "C:/Users/nikos/Documents/phd/datasets/synthetic_checkins_CPM.txt");
    cout << "Grid creation done!" << endl;  
      
       
    cout << "Creating Grid ..." << endl;
  //  createSpatialDataset(&grid);
    cout << "Grid creation done!" << endl; 
  */  
   
/*   
    float sum = 0.0;
   
    for(int i = 1; i < 10; i++){ 
   
            t1 = clock();
            grid.getNextNNCPM((X/2)*DELTA, (Y/2)*DELTA, 0);
            t2 = clock();

            float diff = ((float)t2 - (float)t1) / 1.0F;
            sum += diff;
    }
    float avg = sum/10000.0F;
    cout << "\n ========= Time avg: " << avg/CLOCKS_PER_SEC << "\n";
    cout << "\n ========= Time sum: " << sum/CLOCKS_PER_SEC << "\n";
*/   
/*    float sum = 0.0;
   
    for(int i = 1; i < 2; i++){
   
            t1 = clock();
            knn = grid.getkNNCPM((X/2)*DELTA, (Y/2)*DELTA, 500);
            t2 = clock();
            float diff = ((float)t2 - (float)t1) / 1.0F;
            sum += diff;
    }
    
    std::list<Point>::iterator iter;
    float avg = sum/1.0F;
    cout << "\n ========= Time avg: " << avg/CLOCKS_PER_SEC << "\n";
    cout << "\n ========= Time sum: " << sum/CLOCKS_PER_SEC << "\n";
 
    cout << "Closest users to Q" << endl;
 */
   //  for (iter = knn.begin(); iter != knn.end(); iter++){ 
   //      Point tmp = *iter;
   //      tmp.printDetails();
   //  }
    
    
/*    
    cout << "Creating Social Graph ..." << endl;
    createSocialGraph(&sg);
    cout << "Social Graph creation done!" << endl;
    
    //sg.printSocialGraphToFile();
    
    cout << "Spatial Expansion Approach" << endl;
    
    
    // na tis vazw orisma ena group* kai na epistrefei auto
   queryExpansionApproach((X/2)*DELTA, (Y/2)*DELTA, 2, &grid, &sg);

    cout << "Get M-1 Closest Friends Approach" << endl;
    
    getM1ClosestFriendsApproach((X/2)*DELTA, (Y/2)*DELTA, 2, &grid, &sg);

    cout << "==== GetNextNN \n";

*/

/*    

    list<Point> cf = getClosestFriendsToQ((X/2)*DELTA, (Y/2)*DELTA, 15, 3, &grid, &sg);
    
    std::list<Point>::iterator iter;
 
 cout << "Closest Friends to Q" << endl;
 
     for (iter = cf.begin(); iter != cf.end(); iter++){ 
         Point tmp = *iter;
         tmp.printDetails();
     }
*/    





