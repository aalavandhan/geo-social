#include "../headersMemory.h"

InvertedIndexGen::InvertedIndexGen() {
    // Default constructor.
}

// The destructor iterates over each file index element deletes it.
InvertedIndexGen::~InvertedIndexGen() {
    map<string, FileIndex*>::iterator it = idx.begin();
    // iterate over each map element pair.
    while (it != idx.end()) {
        FileIndex* fi = it->second;
        // Now, delete each file index.
        while (fi != NULL) {
            FileIndex* p = fi;
            fi = fi->getNext();
            delete p;
        }
        it++;
    }
    // Next, delete all map entries.
    idx.clear();
}


// Looks up a word in the inverted index.
FileIndex* InvertedIndexGen::lookup(const string& word) {
    return idx[word];
}


// Inserts a word into the inverted index.
// Linked List with tf-idf possiblity
void InvertedIndexGen::insertAsLinkedList(const string& word, int occurrences, int user_id) {
	// cout<<"Inserting word : "<<word<<" occurrences: "<< occurrences<<" user_id: "<< fcnt<<endl;
    if(idx.find(word) == idx.end()) {                // word not seen
        FileIndex* newEntry = new FileIndex(user_id, occurrences);  // add new pair to idx map
        idx.insert(pair<string, FileIndex*>(word, newEntry) );
		IDF.insert(pair<string, int>(word, occurrences) );
        return;
    }
    else {                                           // word has been seen
        unordered_map<string, int>::iterator it = IDF.find(word); //update Inverse document frequencies
		it->second = it->second + occurrences;

		FileIndex* curr = lookup(word);
        while(curr->getUserId() != user_id && curr->getNext() != NULL){  // iterate through word's FileIndex objects
            curr = curr->getNext();
        }
		if(curr->getNext() == NULL){
			FileIndex* addIndex = new FileIndex(user_id, occurrences);	 	// add new FileIndex pointer to value list
			curr->setNext(addIndex);
		}
		else{
			curr->incrTF(occurrences);
		}
    }
}

// Returns true if c is an alpha character.
bool alpha(char c) {
	return isalpha(c);
}

// Returns true if c is not an alpha character.
bool not_alpha(char c) {
	return !isalpha(c);
}

//
// This method splits the string str into a vector of strings.  That is, we
// split a sentence into words.
//
vector<string> split(const string& str) {
	vector<string> ret;
	string::const_iterator i = str.begin();
	while (i != str.end()) {
		// Ignore unrecognized characters (non-alpha):
		i = find_if(i, str.end(), alpha);

		// Find the next alpha word:
		string::const_iterator j = find_if(i, str.end(), not_alpha);

		// Copy the characters in [i, j)
		if (i != str.end())
			ret.push_back(string(i, j));

		i = j;
	}
	return ret;
}


int InvertedIndexGen::loadFile(const string& file) {
	ifstream infile(file.c_str());
	int count = 0;
	if (infile) {
		string line;
		while (getline(infile, line)) {
			char *bigBuffer = new char[line.size()+1];
			bigBuffer[line.size()]=0;
			memcpy(bigBuffer, line.c_str(),line.size());
			
			char *split;
			char *saveptr;
			int i =0;
			split = strtok_r(bigBuffer, "|",&saveptr);
			string word;
			int user_id;
			
			while(split != NULL){
				//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
				if(strlen(split) > 0){
					if(i==0){
						user_id = atoi(split);
						
					}
					else if(i % 2 == 1){
						string temp(split);
						word = temp;
					}
					else{
						int occurrences = atoi(split);
						// cout<<"Inserting word : "<<word<<" occurrences: "<< occurrences<<" user_id: "<< fcnt<<endl;
						insertAsLinkedList(word, occurrences, user_id);  // for linked list implementation
					}
				}
				split = strtok_r(NULL, "|",&saveptr);
				i++;
			}
			
			count++;
		}
		cout<< "Parsed line number in keyword file: "<<count<<endl;
		
		return 0;
	}
	else {
		cerr << "can't open file " << file << endl;
		return -1;
	}
}

//load user descriptions directly
int InvertedIndexGen::loadFileToIDX(const string& file) {
	ifstream infile(file.c_str());
	int count = 0;
	double total_occurrences = 0;
	if (infile) {
		string line;
		while (getline(infile, line)) {
			char *bigBuffer = new char[line.size()+1];
			bigBuffer[line.size()]=0;
			memcpy(bigBuffer, line.c_str(),line.size());
			
			char *split;
			char *saveptr;
			int i =0;
			split = strtok_r(bigBuffer, "|",&saveptr);
			string word;
			int user_id;
			
			
			while(split != NULL){
				//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
				if(strlen(split) > 0){
					if(i==0){
						user_id = atoi(split);
						
					}
					else if(i % 2 == 1){
						string temp(split);
						word = temp;
					}
					else{
						int term_frequency = atoi(split);
						
						// if(user_id == 1){
							// cout<<"word: "<<word<<" TF: "<<term_frequency<<endl;
						// }

						// auto idxVEC_it = idxOfVec.find(word);
						auto idxSET_it = idxOfSet.find(word);
						// if(idxVEC_it != idxOfVec.end() && idxSET_it!=idxOfSet.end()){
						if(idxSET_it!=idxOfSet.end()){
							// (idxVEC_it->second)->push_back(tf_pair(user_id, term_frequency) );
							(idxSET_it->second)->insert(tf_pair(user_id, term_frequency) );
						}
						else{
							multiset<tf_pair, pair_descending_frequency>* posting_list_SET = new multiset<tf_pair, pair_descending_frequency>();
							// vector<tf_pair>* posting_list_VEC = new vector<tf_pair>();
						
							// posting_list_VEC->push_back(tf_pair(user_id, term_frequency) );
							posting_list_SET->insert(tf_pair(user_id, term_frequency) );
							
							// if(word == "rock")
							// cout<<"insert rock word: "<<word<<endl;
							idxOfSet.insert (make_pair(word, posting_list_SET));
							// idxOfVec.insert (make_pair(word, posting_list_VEC));
						}
						
						
						auto idf_iter = IDF.find(word);
						if(idf_iter!= IDF.end()){
							idf_iter->second = idf_iter->second + 1;
						}
						else{
							IDF.insert(pair<string, int>(word, 1) );
						}
						
						
						//update document length;
						auto iter = docLenMap.find(user_id);
						double tf_w = 1+log10(term_frequency);
						
						if(iter==docLenMap.end()){
							docLenMap.insert(make_pair(user_id, tf_w*tf_w));
						}
						else{
							iter->second = iter->second + (tf_w*tf_w);
						}						
						++total_occurrences;
					}
				}
				split = strtok_r(NULL, "|",&saveptr);
				i++;
			}
			
			count++;
			
		}
		
		cout<<"Number of keywords in idxOfSet: "<<idxOfSet.size()<<endl;
		cout<< "Parsed lines in keyword file: "<<count<<endl;
		
		/*
		//iterate through the entire index
		//create running stats summary for each keyword
		for(auto iter = idxOfVec.begin(); iter!=idxOfVec.end();++iter){
			string word = iter->first;
			vector<tf_pair>* posting_list_VEC = iter->second;
			
			running_stat* rs = new running_stat();
			rs->word = word;
			rs->probablity_non_zero = posting_list_VEC->size()/total_occurrences;
			
			double idf_w = log10(DATASET_SIZE/(double)(posting_list_VEC->size()));
			//for each user in posting list , calculate impact, and its stats
			for(auto it = posting_list_VEC->begin();it!=posting_list_VEC->end();++it){
				tf_pair user = *it;
				int user_id = user.getId();
				int term_frequency = user.getTF();
				
				auto it_dl = docLenMap.find(user_id);
				if(it_dl!=docLenMap.end()){
					double partial_tfidf = ((1+log10(term_frequency)) * idf_w ) / (double) (sqrt(it_dl->second));
					rs->add(partial_tfidf);
				}
			}
			keywordSummaries.insert(make_pair(word,rs));
		}
		cout<< "Create running stats for "<< idxOfVec.size() << " keywords"<< endl;
		*/
		return 0;
	}
	else {
		cerr << "can't open file " << file << endl;
		return -1;
	}
}

//load user descriptions directly
int InvertedIndexGen::loadPOIToIDX(const string& file) {
	ifstream infile(file.c_str());
	int count = 0;
	if (infile) {
		string line;
		while (getline(infile, line)) {
			char *bigBuffer = new char[line.size()+1];
			bigBuffer[line.size()]=0;
			memcpy(bigBuffer, line.c_str(),line.size());
			
			char *split;
			char *saveptr;
			int i =0;
			split = strtok_r(bigBuffer, "|",&saveptr);
			string word;
			int user_id;

			while(split != NULL){
				//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
				if(strlen(split) > 0){
					if(i==0){
						user_id = atoi(split);
					}
					else if(i % 2 == 1){
						string temp(split);
						word = temp;
					}
					else{
						int term_frequency = atoi(split);
						auto idxSET_it = idxOfSet_POI.find(word);
						if(idxSET_it!=idxOfSet_POI.end()){
							(idxSET_it->second)->insert(tf_pair(user_id, term_frequency) );
						}
						else{
							multiset<tf_pair, pair_descending_frequency>* posting_list_SET = new multiset<tf_pair, pair_descending_frequency>();
							posting_list_SET->insert(tf_pair(user_id, term_frequency) );
							idxOfSet_POI.insert(make_pair(word, posting_list_SET));
						}

						// auto idf_iter = IDF_POI.find(word);
						// if(idf_iter!= IDF_POI.end()){
							// idf_iter->second = idf_iter->second + 1;
						// }
						// else{
							// IDF_POI.insert(pair<string, int>(word, 1) );
						// }
						
						
						//update document length;
						auto iter = docLenMap_POI.find(user_id);
						double tf_w = 1+log10(term_frequency);
						
						if(iter==docLenMap_POI.end()){
							docLenMap_POI.insert(make_pair(user_id, tf_w*tf_w));
						}
						else{
							iter->second = iter->second + (tf_w*tf_w);
						}						
					}
				}
				split = strtok_r(NULL, "|",&saveptr);
				i++;
			}
			
			count++;
			
		}
		cout<< "Parsed lines in POI keyword file: "<<count<<endl;
		return 0;
	}
	else {
		cerr << "can't open file " << file << endl;
		return -1;
	}
}



//load as set of tf_pair
int InvertedIndexGen::loadFileWithTF(const string& file) {
	ifstream infile(file.c_str());
	int count = 0;
	double total_occurrences = 0;
	if (infile) {
		string line;
		while (getline(infile, line)) {
			char *bigBuffer = new char[line.size()+1];
			bigBuffer[line.size()]=0;
			memcpy(bigBuffer, line.c_str(),line.size());
			
		
			char *split;
			char *saveptr;
			int i = 0;
			split = strtok_r(bigBuffer, " ",&saveptr);
			string word;
			int user_id ; //frequency;
			unsigned int size;
			multiset<tf_pair, pair_descending_frequency>* posting_list_SET = new multiset<tf_pair, pair_descending_frequency>();
			//string res;
			vector<tf_pair>* posting_list_VEC = new vector<tf_pair>();
			
			while(split != NULL){
				//		   cout << i << ") " << split << " \t valid_end = " << valid_end << endl;
				//frequency = 0;
				if(strlen(split) > 0){
					if(i == 0){
						string temp(split);
						word = temp;
						//res = word;
					}
					else if(i == 1){
						size = (unsigned int) (atoi(split));
					}
					else if(i%2==0){
						user_id = atoi(split);
					}
					else if(i%2==1){
						int term_frequency = atoi(split);
						//frequency ++;
						
						posting_list_VEC->push_back(tf_pair(user_id, term_frequency) );
						posting_list_SET->insert(tf_pair(user_id, term_frequency) );
						//res += " " + util.int_to_string(user_id) + " " + util.int_to_string(term_frequency);
						
						//update document length;
						auto iter = docLenMap.find(user_id);
						double tf_w = 1+log10(term_frequency);
						
						if(iter==docLenMap.end()){
							docLenMap.insert(make_pair(user_id, tf_w*tf_w));
						}
						else{
							iter->second = iter->second + (tf_w*tf_w);
						}
						
						++total_occurrences;
						
					}
				}
				split = strtok_r(NULL, " ",&saveptr);
				i++;
			}
			
			if(posting_list_VEC->size() != size || posting_list_SET->size() != size){
				cerr << "posting list size mismatch " << file << endl;
				return -1;
			}
				
			// cout<<word<<"|"<<size<<endl;
			//cout<<"inserting: " << res << endl;
			
			IDF.insert(pair<string, int>(word, size) );
			idxOfSet.insert (make_pair(word, posting_list_SET));
			// idxOfVec.insert (make_pair(word, posting_list_VEC));
			

			
			count++;
		}
		
		cout<<"Number of keywords in idxOfSet: "<<idxOfSet.size()<<endl;
		
		cout<< "Parsed lines in keyword file with TFs: "<< count << endl;
		
		/*
		//iterate through the entire index
		//create running stats summary for each keyword
		// for(auto iter = idxOfVec.begin(); iter!=idxOfVec.end();++iter){
		for(auto iter = idxOfVec.begin(); iter!=idxOfVec.end();++iter){
			string word = iter->first;
			vector<tf_pair>* posting_list_VEC = iter->second;
			
			running_stat* rs = new running_stat();
			rs->word = word;
			rs->probablity_non_zero = posting_list_VEC->size()/total_occurrences;
			
			double idf_w = log10(DATASET_SIZE/(double)(posting_list_VEC->size()));
			//for each user in posting list , calculate impact, and its stats
			for(auto it = posting_list_VEC->begin();it!=posting_list_VEC->end();++it){
				tf_pair user = *it;
				int user_id = user.getId();
				int term_frequency = user.getTF();
				
				auto it_dl = docLenMap.find(user_id);
				if(it_dl!=docLenMap.end()){
					double partial_tfidf = ((1+log10(term_frequency)) * idf_w ) / (double) (sqrt(it_dl->second));
					rs->add(partial_tfidf);
				}
			}
			keywordSummaries.insert(make_pair(word,rs));
		}
		cout<< "Create running stats for "<< idxOfVec.size() << " keywords"<< endl;
		*/
		return 0;
	}
	else {
		cerr << "can't open file " << file << endl;
		return -1;
	}
}

/*
Estimating the Selectivity of tf-idf based Cosine Similarity Predicates  ---  Sandeep Tata Jignesh M. Patel : University of Michigan --- SIGMOD RECORD 2007
Algorithm ES
*/
void InvertedIndexGen::estimateTFIDFdistribution(vector<string>* terms){
	//TODO
	//Algorithm ES(query,threshold,summary)
	// 1. Construct the tf.idf vector u for the query.

	double query_length = 0;
	map<string, double> w_idf_map;
	// calculate query_length_squared;
	for(auto itt = terms->begin(); itt!=terms->end();++itt){
		int idf = IDF[*itt];
		double idf_w = log10(DATASET_SIZE/(double)(idf));
		w_idf_map.insert((make_pair(*itt,idf_w)));
		query_length += idf_w*idf_w;
		
	}
	query_length = sqrt(query_length);
	
	for(auto it = w_idf_map.begin(); it!=w_idf_map.end();++it){
		it->second = it->second/query_length;
	}
	
	double alpha = 1;
	double uES =0,sES=0,pzES=0;
	
	
	for(auto it = w_idf_map.begin(); it!=w_idf_map.end();++it){
	
		double idf_w = it->second;
		auto ks_it = keywordSummaries.find(it->first);
		running_stat* rs = ks_it->second;
		double mean = rs->mean();
		double sd = rs->sd();
		double probablity_non_zero = rs->probablity_non_zero;
		
		// 2. Compute µES = αΣN (µi × Ci × ui)
		uES += mean * idf_w * probablity_non_zero;
		// 3. Compute σES = βΣN (σi × Ci × ui)
		sES += sd * idf_w * probablity_non_zero;
		
		// 4. Compute over nonzero ui :
		// 5. nzES = 1 − (ΠNi=1(1 − Ci))
		pzES += 1-probablity_non_zero;
	}
		
	double nzES = 1 - pzES;
	
	
}



//return the list of users from the array based implementation
multiset<tf_pair, pair_descending_frequency>* InvertedIndexGen::getUsersWithTFfromSET(string word){
	// cout<<"idxOfSet size : "<<idxOfSet.size()<<endl;
	
	// for(auto it = idxOfSet.begin(); it!= idxOfSet.end(); it++){
		// cout<<it->first<<" has size: "<< it->second->size()<<endl;
	// }
	
	auto it = idxOfSet.find(word);
	if(it != idxOfSet.end()){
		return it->second;
	}
	else{
		cout<<"Query word not in vocabulary: "<<word<<endl;
		return NULL;
	}
}

multiset<tf_pair, pair_descending_frequency>* InvertedIndexGen::getPOIWithTFfromSET(string word){

	if(idxOfSet_POI.find(word) != idxOfSet_POI.end()){
		return idxOfSet_POI[word];
	}
	else{
		cout<<"Query word not in vocabulary"<<endl;
		return NULL;
	}
}

/*
vector<tf_pair>* InvertedIndexGen::getUsersWithTFfromVEC(string word){

	if(idxOfVec.find(word) != idxOfVec.end()){
		return idxOfVec[word];
	}
	else{
		cout<<"Query word not in vocabulary"<<endl;
		return NULL;
	}
}
*/


double InvertedIndexGen::calculateTFIDFfromSET(string term, int user_id){

	multiset<tf_pair, pair_descending_frequency>* posting_list = getUsersWithTFfromSET(term);
	 
	multiset<tf_pair, pair_descending_frequency>::iterator it = posting_list->find(tf_pair(user_id,1));
	if (it != posting_list->end()) {
		tf_pair user = *it;
		int term_frequency = user.getTF();
		double idf = log10(DATASET_SIZE/(1 + posting_list->size()));
		return term_frequency * idf;
	}
	else{
		return 0;
	}
}



string InvertedIndexGen::toTFSetString() {
	typedef unordered_map<string, multiset<tf_pair, pair_descending_frequency>* >::iterator it_type;
	
	string res = "";
	for(it_type iterator = idxOfSet.begin(); iterator != idxOfSet.end(); iterator++) {
		string word = iterator->first;
		multiset<tf_pair, pair_descending_frequency>* posting_list = iterator->second;
		int numOfUsers = posting_list->size();
		res += word + " " + util.int_to_string(numOfUsers)+ "\n";
	}
	return res;
}




//load as integer array with no information about term frequencies
int InvertedIndexGen::loadFileNoTF(const char* file){
	
	ifstream fin(file);
    if (!fin){
        cout << "Cannot open keywords file " << file << endl;
		cout << "Error: " << strerror(errno);
        return -1;
    }

    int size;
	string word;
	
    unsigned int times = 0;
    int totalusers = 0;
    cout << "Loading the Keyword Map from " << file << endl;
    Value* entry;
    while(fin){ //NUMOFWORDS

        fin >> word >> size;
        if (! fin.good()){
            cout << "fin is not good: word = " << word << endl;
            continue;
        }
		
		int id = 999999; // garbage value
        entry = new Value(size, id);

        int* list = (int*) malloc(sizeof(int)*size);
        totalusers+=sizeof(int)*size;
        for(int i = 0; i<size; i++){
            fin >> list[i];
        }
        entry->setList(list, size);

        idxOfArray.insert(make_pair(word, entry));
        times++;
    }
    fin.clear();
    fin.close();
    cout << "loaded keywords = " << times << endl;
    cout << "totalusers = " << (totalusers/(1024)) << "KB" << endl;
    return 0;
}

//return the list of users from the array based implementation
void InvertedIndexGen::getUsersNoTF(string word, int*& users, unsigned int &numOfUsers){

    Value* v = NULL;
    map<string, Value*>::iterator it = idxOfArray.find(word);
    if(it != idxOfArray.end()){
        //element found;
        v = it->second;
    }
    else {
        numOfUsers = 0;
        users = (int*)malloc(sizeof(int)*numOfUsers);
    }

    if(v!=NULL){
        numOfUsers = v->getListSize();
        int* tmp = v->getList();
        users = (int*)malloc(sizeof(int)*numOfUsers);

        for(unsigned int i=0; i< numOfUsers; i++){
            users[i] = tmp[i];
        }
        free(tmp);
    }

}


//
// This method reads the words in the provided file into the vector v.
//
int InvertedIndexGen::readWords(const string& file, vector<string>& v) {
	ifstream infile(file.c_str());
	if (infile) {
		string line;
		while (getline(infile, line)) {
			vector<string> words = split(line);
			v.insert(v.end(), words.begin(), words.end());
		}
		return 0;
	}
	else {
		cerr << "can't open file " << file << endl;
		return -1;
	}
}

//
// This method converts the FileIndex list fi into a set of
// integers representing the file indexes.
//
void to_set(set<int>& s, FileIndex* fi) {
    FileIndex* p = fi;
    while (p != NULL) {
        s.insert(p->getUserId());
        p = p->getNext();
    }
}

//
// Returns the string representation of the inverted index.
//
string InvertedIndexGen::toSimpleString() {
    set<int> indexes;
    string   res = "";
    map<string, FileIndex*>::iterator it = idx.begin();
    
    while (it != idx.end()) {
        to_set(indexes, it->second);
		res += it->first + " " + util.int_to_string(indexes.size());
        for (set<int>::iterator it2 = indexes.begin(); it2 != indexes.end(); ++it2) {
            res += " " + util.int_to_string(*it2) ;
        }    
        res += "\n";    
        indexes.clear();
        it++;
    }

    return res;
}

//
// This method converts the FileIndex list fi into an array of
// integers representing the users followed by their term frequency.
//
void to_TFArray(vector<int>& s, FileIndex* fi) {
    FileIndex* p = fi;
    while (p != NULL) {
        s.push_back(p->getUserId());
		s.push_back(p->getTF());
        p = p->getNext();
    }
}

//
// Returns the string representation of the inverted index with term frequencies.
//
string InvertedIndexGen::toTFString() {
    vector<int> values;
    string res = "";
    map<string, FileIndex*>::iterator it = idx.begin();

	while (it != idx.end()) {
        to_TFArray(values, it->second);
		if(values.size()%2 != 0 ){
			cout << "ERROOOORRR"<<endl;
			break;
		}
		res += it->first + " " + util.int_to_string((values.size()/2));
        for (vector<int>::iterator it2 = values.begin(); it2 != values.end(); ++it2) {
            res += " " + util.int_to_string(*it2) ;
        }    
        res += "\n";    
        values.clear();
        it++;
    }

    return res;
}


string InvertedIndexGen::toNoTFString() {
	typedef map<string, Value*>::iterator it_type;
	string res = "";
	Value* v = NULL;
	for(it_type iterator = idxOfArray.begin(); iterator != idxOfArray.end(); iterator++) {
		string word = iterator->first;
		v = iterator->second;
		int numOfUsers = v->getListSize();
		res += word + " " + util.int_to_string(numOfUsers)+ "\n";
	}
	return res;
}




int InvertedIndexGen::numberOfWords() {
	return idx.size();
}
