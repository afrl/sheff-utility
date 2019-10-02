/// Unit test for Hash.h

#include<iostream>
#include "utility/Hash.h"

#include <assert.h>
#include <string>

using namespace std; 

int main()
{

  cout << "Test: Hash" << endl << endl; 
  hash<string> str_hash; 
  hash<int> int_hash; 
  string my_str = "my string";

  cout << "Check unsorted hashes match... "; 
  utility::hash util_hash;
  assert(str_hash(my_str) == util_hash(my_str,0)); 
  size_t xor_hash = int_hash(5) ^ str_hash(my_str); 
  assert(xor_hash == util_hash(5, my_str)); 
  assert(xor_hash == util_hash(std::pair<int,string>(5, my_str))); 
  cout << "passed" << endl; 

  cout << "Check sorted hashes match... "; 
  utility::hash_sorted util_sorthash; 
  // These must be compatible types for sort comparison. The next line would give
  // a type mismatch error: 
  //assert (xor_hash ==  util_sorthash(std::pair<int,string>(5, my_str))); 
  assert (util_sorthash(std::pair<string,string>("abc", "def")) == \
          util_sorthash(std::pair<string,string>("def", "abc"))); 
  assert (util_sorthash(7.3,5) == util_sorthash(5,7.3)); 
  cout << "passed" << endl; 

  return 0; 
}
