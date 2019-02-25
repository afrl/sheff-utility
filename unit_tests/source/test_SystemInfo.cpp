/// Unit test for SystemInfo.h

#include<iostream>
#include "utility/SystemInfo.h"

using namespace std; 

int main()
{

  cout << "Test: SystemInfo" << endl << endl; 

  utility::SystemInfo sys;
  
  cout << utility::SystemInfo::OS << endl; 
  cout << utility::SystemInfo::compiler << endl; 
  cout << utility::SystemInfo::compiler_version << endl; 
  cout << sys.get_compile_info() << endl; 
 
  return 0; 
}
