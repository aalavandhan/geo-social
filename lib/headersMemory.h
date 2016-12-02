#include "headers.h"

// REAL

// Grid Set Up


//YELP LAS VEGAS
// #define DATASET_SIZE 40298
// #define POI_SIZE 12773
// #define MAX_X -114.927464
// #define MAX_Y 36.328529
// #define MIN_Y 35.996499
// #define MIN_X -115.339468



//YELP PHOENIX
// #define DATASET_SIZE 30057
// #define POI_SIZE 16154
// #define MAX_Y 33.859067
// #define MIN_Y 33.219214
// #define MAX_X -111.711288
// #define MIN_X -112.495437

#define ESTIM_ALPHA 0.331577570295903
#define ESTIM_BETA 0.306589909581214


extern double DATASET_SIZE;
extern double POI_SIZE;

extern double MIN_X;
extern double MIN_Y;
extern double MAX_X;
extern double MAX_Y;
extern double DELTA_X ;
extern double DELTA_Y ;

// GSKI PARAMETERS
// extern int X;   
// extern int Y;
// extern int HG_HEIGHT;
// extern int R_FAN;
// extern int HG_FANOUT;



// GSKI PARAMETERS for running example
// #define X 4 // table[X][Y]   
// #define Y 4
// #define HG_HEIGHT 2
// #define R_FAN 2
// #define HG_FANOUT 4


// GSKI PARAMETERS 
// #define X 81 // table[X][Y]   
// #define Y 81
// #define HG_HEIGHT 4
// #define R_FAN 3
// #define HG_FANOUT 9

// GSKI PARAMETERS 
#define X 256 // table[X][Y]   
#define Y 256
#define HG_HEIGHT 4
#define R_FAN 4
#define HG_FANOUT 16


// GSKI PARAMETERS 
// #define X 625 // table[X][Y]   
// #define Y 625
// #define HG_HEIGHT 4
// #define R_FAN 5
// #define HG_FANOUT 25


// GSKI PARAMETERS 
// #define X 1296 // table[X][Y]   
// #define Y 1296
// #define HG_HEIGHT 4
// #define R_FAN 6
// #define HG_FANOUT 36


//------------------------------------

// GSKI PARAMETERS 
// #define X 243 // table[X][Y]   
// #define Y 243
// #define HG_HEIGHT 5
// #define R_FAN 3
// #define HG_FANOUT 9

// GSKI PARAMETERS 
// #define X 1024 // table[X][Y]   
// #define Y 1024
// #define HG_HEIGHT 5
// #define R_FAN 4
// #define HG_FANOUT 16


// GSKI PARAMETERS 
// #define X 3125 // table[X][Y]   
// #define Y 3125
// #define HG_HEIGHT 5
// #define R_FAN 5
// #define HG_FANOUT 25





#define CONV_PERCENTILE 100/DATASET_SIZE

// for hierarchical grid
// #define X 1536 // table[X][Y]
// #define Y 1536a

//YELP COMPLETE
// #define DATASET_SIZE 58062
// #define X 200 // table[X][Y]
// #define Y 200
// #define MAX_X -3.0562661
// #define MAX_Y 55.9816255427668
// #define MIN_X -115.369725427566
// #define MIN_Y 32.8768662
// #define VOCAB_SIZE 930
//AUSTIN
// #define DATASET_SIZE 5868
// #define X 100 // table[X][Y]
// #define Y 100
// #define MAX_X -97.5564050674
// #define MAX_Y 30.4098181886
// #define MIN_X -97.8997278214
// #define MIN_Y 30.1313942671

//GOWALLA
// #define DATASET_SIZE 1141
// #define X 1000 // table[X][Y]
// #define Y 1000

// #define MAX_X -97.63822406
// #define MAX_Y 30.4087502667
// #define MIN_X -97.8879422167
// #define MIN_Y 30.1541876667


//VLDB Experiments star-graph
//#define DATASET_SIZE 12652
//#define X 3000 // table[X][Y]
//#define Y 1000

//#define MIN_Y -37.8
//#define MIN_X -158
//#define MAX_Y 64
//#define MAX_X 175.0


//#define MIN_Y -37.8
//#define MIN_X -158
//#define MAX_Y 64
//#define MAX_X 175.0

//Experiments NNTopK
//#define DATASET_SIZE 5202
//#define X 1000 // table[X][Y]
//#define Y 1000

//#define MIN_Y 40.5598
//#define MIN_X -74.1994
//#define MAX_Y 40.8996
//#define MAX_X -73.691

// Dataset
//#define DATASET_SIZE 12652

// dataset scalability
//#define DATASET_SIZE 5000000
//#define X 300 // table[X][Y]   
//#define Y 300
//#define MIN_X -37.8
//#define MIN_Y -158.0
//#define MAX_X 64.0
//#define MAX_Y 175.0


//#define DATASET_SIZE 10000

/// @brief The usual PI/180 constant
#define DEG_TO_RAD 0.017453292519943295769236907684886

/// @brief Earth's quatratic mean radius for WGS-84
#define EARTH_RADIUS_IN_KILOMETERS 6371
#define EARTH_CIRCUMFERENCE 40075.017

// #define DELTA_X ((MAX_X - MIN_X)/ (X-1))
// #define DELTA_Y ((MAX_Y - MIN_Y)/ (Y-1))

#define DELTA_XY (sqrt(DELTA_X*DELTA_X + DELTA_Y*DELTA_Y))

//bloom filter param
#define CAPACITY 2000
#define ERROR_RATE 0.1

/*
 // DENSE

// Grid Set Up
#define X 300 // table[X][Y]   
#define Y 300
#define MIN_X 40             
#define MIN_Y -73
#define MAX_X 41
#define MAX_Y -72
#define DELTA_X ((MAX_X - MIN_X)/ (X-1)) 
#define DELTA_Y ((MAX_Y - MIN_Y)/ (Y-1))

*/

/*
 //SPARSE

// Grid Set Up
#define X 300 // table[X][Y]   
#define Y 300
#define MIN_X 30             
#define MIN_Y -84
#define MAX_X 50
#define MAX_Y -62
#define DELTA_X ((MAX_X - MIN_X)/ (X-1)) 
#define DELTA_Y ((MAX_Y - MIN_Y)/ (Y-1))


// Dataset
#define DATASET_SIZE 10000
*/

// Cell type, it is used for the heaps
#define CELL 0
#define RECTANGLE 1
#define POINT 2

// QuadTreeNode types used for heap
#define QTN 1
#define USER 2

// CPM directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#include "bloom/murmur.h"
#include "bloom/dablooms.h"


#include "SPOs/MemoryMap/SPOs.h"
#include "GPOs/MemoryGrid/grid/QuadTreeNode.h"
#include "GPOs/MemoryGrid/grid/Cell.h"

#include "GPOs/MemoryGrid/grid/IncrVisitor.h"
#include "GPOs/MemoryGrid/grid/Visitor.h"
#include "GPOs/MemoryGrid/grid/Grid.h"


//#include "SPOs/MemoryMap/pch.h"
//#include "GPOs/MemoryGrid/headers.h"


#include "GPOs/MemoryGrid/GPOs.h"


#include "GSKqueries/topkGSK.h"

// #include "SPOs/MemoryMapWeighted/Pair.h"
#include "SPOs/MemoryMapWeighted/User.h"
#include "SPOs/MemoryMapWeighted/Graph.h"



