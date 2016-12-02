#include "../headers.h"

//
// Creates a new file index given the index i.
//
FileIndex::FileIndex(int _user_id, int _term_frequency) {
  user_id = _user_id;
  next  = NULL;
  term_frequency  = _term_frequency;
}

//
// Returns the next file index in this list.
//
FileIndex* FileIndex::getNext() {
  return next;
}

//
// Sets the next file index in this list.
//
void FileIndex::setNext(FileIndex* n) {
  next = n;
}

//
// Returns the user_id.
//
int FileIndex::getUserId() {
  return user_id;
}

//
// Returns the occurrences.
//
int FileIndex::getTF() {
  return term_frequency;
}

//
// Sets the number of occurrences of the word in this file index.
//
void FileIndex::setTF(int _term_frequency) {
  term_frequency = _term_frequency;
}

void FileIndex::incrTF(int _increment) {
  term_frequency = term_frequency + _increment;
}
