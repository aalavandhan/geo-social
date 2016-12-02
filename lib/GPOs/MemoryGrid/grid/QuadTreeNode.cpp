#include "../../../headersMemory.h"

QuadTreeNode::QuadTreeNode(double _x1, double _y1, double _x2, double _y2, unsigned int _level, unsigned int _length, QuadTreeNode* pParent)
{	
	numOccupants=0;
	numPointsOfInterest=0;
	hasChildren= false;
	pParentNode =pParent;
	level =_level;
	length =_length;

	x1 = new double;
    x2 = new double;                  //  |-----X2
    y1 = new double;                  //  |     |
    y2 = new double;
	*x1 = _x1; *x2 = _x2; *y2 = _y2; *y1 =_y1;
	// idx = new unordered_map<string, int >();
	idxImpact = new unordered_map<string, double >();
	P_idxImpact = new unordered_map<string, double >();
	max_degree = 0;
	max_tfidf = 0;
	c_minDist = 0;
	c_maxDist = 0;
	vnp_score = 0;
	real_score = 0;
	diversity_score = 0;
	// cout<<"\t\t\t Creating Quad Tree node at level: "<<level<<" length: "<<length << " |  x1 "<<*x1<<" y1 "<<*y1<<" x2 "<<*x2<<" y2 "<<*y2<<" pParent = "<<pParent<<endl;	
}

QuadTreeNode::QuadTreeNode(QuadTreeNode* _qtn)
{	
	hasChildren = _qtn->hasChildren;
	pParentNode = _qtn->pParentNode;
	level = _qtn->level;
	length = _qtn->length;
	node_type = _qtn->node_type;
	
	x1 = new double;
    x2 = new double;                  //  |-----X2
    y1 = new double;                  //  |     |
    y2 = new double;
	*x1 = *(_qtn->x1); *x2 = *(_qtn->x2); *y2 = *(_qtn->y2); *y1 = *(_qtn->y1);
	
	//idx = new map<string, int >();
	
	max_degree = _qtn->max_degree;
	max_tfidf = _qtn->max_tfidf;
	c_minDist = _qtn->c_minDist;
	//maybe partial scores are not required
	real_score = _qtn->real_score;
	diversity_score = _qtn->diversity_score;
	bounding_score = _qtn->bounding_score;
	
	cout<<"\t\t\t Copying Quad Tree node at level: "<<level<<" length: "<<length << " |  x1 "<<*x1<<" y1 "<<*y1<<" x2 "<<*x2<<" y2 "<<*y2<<" pParent = "<<pParentNode<<endl;
}

QuadTreeNode::~QuadTreeNode()
{
	// cout<<"DESTROYING NODE TYPE "<<node_type<<endl;
	if(node_type == QTN)
		for(auto it = children->begin();it!=children->end();++it)
				delete *it;
}

void QuadTreeNode::setType(int type){
    node_type = type;
}

int QuadTreeNode::getID(){
    return user_id;
}

void QuadTreeNode::setID(int id){
    user_id = id;
}

int QuadTreeNode::getType(){
    return node_type;
}

double QuadTreeNode::getX1(){ return *x1;}

double QuadTreeNode::getY1(){ return *y1;} 

double QuadTreeNode::getX2(){ return *x2;}

double QuadTreeNode::getY2(){ return *y2;}       

void QuadTreeNode::setTFIDF(double _value){
	max_tfidf = _value;
}

double QuadTreeNode::computeScore(){
	bounding_score = (max_degree/(double)MAXSC) + (1 - c_minDist/(double)MAXDIST) + max_tfidf;
	real_score = bounding_score;
	return bounding_score;
}

double QuadTreeNode::computeMulScore(){
	bounding_score = (max_degree * max_tfidf) / c_minDist;
	real_score = bounding_score;
	return bounding_score;
}

void QuadTreeNode::setPerccentileScore(double _score){
	bounding_score = _score;
}


double QuadTreeNode::computePOIScore(){
	double w_s = 3.0;
	bounding_score = (vnp_score*w_s) + (1 - c_minDist/(double)MAXDIST) + max_tfidf;
	return bounding_score;
}

void QuadTreeNode::setBloomVNP(double _vnp_score){
	vnp_score = _vnp_score;
}


double QuadTreeNode::computeMinDist(double x, double y){

	if(x > *x1 && x < *x2 && y > *y1 && y < *y2){
		c_minDist = 0.0;
		return c_minDist;
	}

    c_minDist = 0.0;

    if (x < *x1){
        c_minDist += std::pow(*x1 - x, 2.0);
    }
    else if (x > *x2){
        c_minDist += std::pow(x - *x2, 2.0);
    }

    if (y < *y1){
        c_minDist += std::pow(*y1 - y, 2.0);
    }
    else if (y > *y2){
        c_minDist += std::pow(y - *y2, 2.0);
    }

    c_minDist = sqrt(c_minDist);

    return c_minDist;
}   


double QuadTreeNode::computeMaxDist(double x, double y){

	//  D(x1,y2)----C(x2,y2)
	//  |	        |
	//  A(x1,y1)----B(x2,y1)
	
	//comment denote position of point NOT of cell

    double dx, dy;
    if (x < *x1) { // left hemisphere
        dx = *x2 - x;
        if (y < *y1) { // lower left quadrant
            dy = *y2 - y; //distance to point C
        }
        else if (y > *y2) { // upper left quadrant
            dy = y - *y1; // distance to point B
        }
        else { // immediate left - can be either point B or C
			dy = max(y - *y1,*y2 - y);
        }
    }
    else if (x > *x2) { // right hemisphere
        dx = x - *x1;
        if (y < *y1) { // lower right quadrant
            dy = *y2 - y; // distance to point D
        }
        else if (y > *y2) { // upper right quadrant
            dy = y - *y1; // distance to point A
        }
        else {
			dy = max(y - *y1,*y2 - y); // immediate right - can be either point A or D.
        }
    }
    else {
        if (y < *y1) { //immediately below - either point C or D
			dx = max(*x2 - x,x - *x1);
			dy = *y2 - y;
        }
        else if (y > *y2) { //immediately above - either point A or B
			dx = max(*x2 - x,x - *x1);
			dy = y - *y1;
        }
        else {
			dx = max(*x2 - x,x - *x1);
			dy = max(*y2 - y,y - *y1); // inside the rectangle or on the edge - can be any corner
        }
    }

	c_maxDist = sqrt(dx * dx + dy * dy);
	
    return c_maxDist;
}   


double QuadTreeNode::getMinDist(){
    return c_minDist;
}  

void QuadTreeNode::setMinDist(double dist){
    c_minDist = dist;
}         