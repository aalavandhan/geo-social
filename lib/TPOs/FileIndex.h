//
// Represents an index to a file.
//
class FileIndex {
 public:
  FileIndex(int, int);
  FileIndex* getNext();
  int getUserId();
  void setNext(FileIndex*);
  void setTF(int);
  void incrTF(int);
  int getTF();

 private:
  int user_id;       // The index of the file a word was found in.
  int term_frequency;  // The the number of occurrences of the word.
  FileIndex* next; // The next file index.
};
