#ifndef HEADER_H
#define HEADER_H


#include <iostream>
#include <sys/time.h>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include <map>

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>
#include <cmath>
#include <fstream>
#include <unistd.h>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <unordered_map>

// --------------------- Server

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <winsock2.h>
//#include <netinet/in.h>

// --------------------- Server
#define EARTH_CIRCUMFERENCE 40075.017

using namespace std;


//VLDB dataset
//#define MAXGROUPSIZE 12652
//#define MAXSC 420                  // maximum number of friends plus one
//#define MAXDIST 52.1636             // maximum distance between to points
//#define MAXDIST 0.021636

//gowalla dataset

// #define MAXGROUPSIZE 1141
// #define MAXSC 71                // maximum number of friends plus one
// #define MAXDIST 39.352          // maximum distance between to points

//austin dataset from Nick
#define MAXGROUPSIZE 5868
extern int MAXSC;                // maximum number of friends plus one
extern double MAXDIST;          // maximum distance between to points
extern int MAXT;
//scallability

// d maxS maxT
// 10 1239 570
// 30 2011 4311
// 50 2931 14354
// 70 3215 21460
// 90 3695 34098


// Scalability
// #define MAXGROUPSIZE 1500000

//d=10
// #define MAXSC 1240               
// #define MAXT 570
//d=30
// #define MAXSC 2012               
// #define MAXT 4311
//d=50
// #define MAXSC 2932               
// #define MAXT 14354
//d=70
// #define MAXSC 3216               
// #define MAXT 21460
//d=90
// #define MAXSC 3696               
// #define MAXT 34098

// #define MAXDIST 0.89831527707  //correspons to 100km


#define EULERS_NUMBER 2.718281828459045
#define BOUNDARY_ERROR 0.00000000000001


#include "utilities/res_point.h"
#include "utilities/Utilities.h"
#include "utilities/Group.h"

#include "GPOs/IGPOs.h"
#include "SPOs/ISPOs.h"

#include "basicGSQueries/BasicGSQueries.h"

#endif