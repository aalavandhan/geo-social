// #include <boost/heap/fibonacci_heap.hpp>

#include <iostream>
#include <sys/time.h>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sstream>
#include <cmath>
#include <cctype>
#include <fstream>
#include <unistd.h>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <cstddef>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>


#include <stdint.h>
#include <stdarg.h>

#include <errno.h>
#include <bitset>

// --------------------- Server

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <winsock2.h>
//#include <netinet/in.h>

// --------------------- Server
#define EARTH_CIRCUMFERENCE 40075.017

#define BOUNDARY_ERROR 0.00000000000001
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
// #define MAXGROUPSIZE 5868
// #define MAXSC 390                // maximum number of friends plus one
// #define MAXDIST 0.48553169497810045          // maximum distance between two points


//yelp dataset
// #define MAXGROUPSIZE 58062
// #define MAXSC 500                // maximum number of friends plus one
// #define MAXDIST 2          // maximum distance between two points

//yelp dataset
#define MAXGROUPSIZE 5868
extern int MAXSC;                // maximum number of friends plus one
extern double MAXDIST;          // maximum distance between to points


#define EULERS_NUMBER 2.718281828459045

#include <boost/math/distributions/inverse_gaussian.hpp>

#include "GPOs/MemoryGrid/grid/Point.h"
#include "utilities/res_point.h"
#include "TPOs/tf_pair.h"
#include "utilities/my_pair.h"
#include "utilities/pair_tuple.h"
#include "utilities/Utilities.h"
#include "utilities/Group.h"
#include "utilities/bloom_filter.hpp"
#include "utilities/running_stat.hpp"
#include "TPOs/FileIndex.h"

#include "GPOs/IGPOs.h"
#include "SPOs/ISPOs.h"


#include "SPOs/MemoryMap/Value.h"
#include "TPOs/InvertedIndexGen.h"

#include "basicGSQueries/BasicGSQueries.h"
#include "complexQueries/mGroup.h"
#include "complexQueries/topkUsers.h"

