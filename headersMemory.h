
#ifndef HEADERSMEMORY_H
#define HEADERSMEMORY_H

#include "headers.h"

// REAL

// Grid Set Up
//scallability
// #define DATASET_SIZE 2000000
// #define X 500 // table[X][Y]
// #define Y 500

//250 km
// #define MAX_X -72.79393586019292
// #define MAX_Y 41.82401035104755
// #define MIN_X -75.30216129645595
// #define MIN_Y 39.579425624113

//64km
// #define MAX_X -73.67130332202277
// #define MAX_Y 41.066482183460764
// #define MIN_X -74.33533012375237
// #define MIN_Y 40.40050397157456

// 100km - official
// #define MAX_X -73.11029266745638
// #define MAX_Y 41.27306291471666
// #define MIN_X -75.41071675619725
// #define MIN_Y 39.36143608785296

// AUSTIN
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
extern double DATASET_SIZE;
#define X 400 // table[X][Y]   
#define Y 400
extern double MIN_X;
extern double MIN_Y;
extern double MAX_X;
extern double MAX_Y;


//#define DATASET_SIZE 10000

/// @brief The usual PI/180 constant
#define DEG_TO_RAD 0.017453292519943295769236907684886

/// @brief Earth's quatratic mean radius for WGS-84
#define EARTH_RADIUS_IN_KILOMETERS 6371
#define EARTH_CIRCUMFERENCE 40075.017

extern double DELTA_X ;
extern double DELTA_Y ;


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

// CPM directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#include "GPOs/MemoryGrid/grid/Point.h"
#include "GPOs/MemoryGrid/grid/Cell.h"
#include "GPOs/MemoryGrid/grid/IncrVisitor.h"
#include "GPOs/MemoryGrid/grid/Visitor.h"
#include "GPOs/MemoryGrid/grid/Grid.h"

//#include "SPOs/MemoryMap/pch.h"
//#include "GPOs/MemoryGrid/headers.h"

#include "GPOs/MemoryGrid/GPOs.h"
#include "SPOs/MemoryMap/Value.h"
#include "SPOs/MemoryMap/SPOs.h"

//#include "SPOs/MemoryMapWeighted/Pair.h"
#include "SPOs/MemoryMapWeighted/User.h"
#include "SPOs/MemoryMapWeighted/Graph.h"

#endif