

// The class definition for the inverted index generator.
class InvertedIndexGen {
 public:
    InvertedIndexGen();
    ~InvertedIndexGen();
    FileIndex* lookup(const std::string&);
    std::string toSimpleString();
	std::string toTFString();
	std::string toNoTFString();
	string toTFSetString();
    int numberOfWords();
	int loadFile(const std::string&);
	int loadFileWithTF(const std::string&);
	int loadFileNoTF(const char* file);
	double calculateTFIDFfromSET(string, int);
	
	multiset<tf_pair, pair_descending_frequency>* getUsersWithTFfromSET(string);
	multiset<tf_pair, pair_descending_frequency>* getPOIWithTFfromSET(string);
	
	unordered_map<int, double> docLenMap;
	unordered_map<int, double> docLenMap_POI;
	
	unordered_map<std::string, int> IDF;
	// unordered_map<std::string, int> IDF_POI;
	
	void getUsersNoTF(string word, int*& users, unsigned int &numOfUsers);
	// vector<tf_pair>* getUsersWithTFfromVEC(string);

	map<string, running_stat*> keywordSummaries;
	void estimateTFIDFdistribution(vector<string>* terms);
	
	int loadFileToIDX(const string& file);
	
	int loadPOIToIDX(const string& file);


	
 private:
    std::map<std::string, FileIndex*> idx;
	unordered_map<string, multiset<tf_pair, pair_descending_frequency>* > idxOfSet;

	unordered_map<string, multiset<tf_pair, pair_descending_frequency>* > idxOfSet_POI;
	
	
	
	// map<string, vector<tf_pair>* > idxOfVec;
	map<string, Value*> idxOfArray;
	
	Utilities util;
    // utility routines
	
    int readWords(const std::string&, std::vector<std::string>&);
    void insertAsLinkedList(const std::string&,int , int);
};
