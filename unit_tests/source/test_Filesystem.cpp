/// Unit test for Filesystem.h

#include<iostream>

#ifdef HAVE_FILESYSTEM
#    include<experimental/filesystem>
#else
#    include "utility/Filesystem.h"
#endif

using namespace std; 
namespace fs = std::experimental::filesystem;

int main()
{

  cout << "Test: Filesystem" << endl << endl; 

  fs::path cwd = fs::current_path();
  cout << "1. cwd: " << cwd << endl;

#if defined(_WIN32) && !defined(__CYGWIN__)
  
  // Test append. NOT smart about paths you provide from strings
  cout << "2. append: " << endl;
  fs::path app("C:\\Users\\jdoe");
  fs::path pabs("X:\\spacy path\\");   // Trailing separator NOT removed. Note slash after X: is REMOVED 
                                       // from parent_path when called on these appended paths
  fs::path rel("\\seps\\");            // Leading separator also NOT removed. With this and previous, get  
                                       // double backslash which is also REMOVED in parent_path.
  fs::path ext("test.txt");
  fs::path invalid("?");               // Should be invalid in Windows filesystem, but no problem to initialize
  fs::path fin("\\a\\");               // If path ends in slash, filename is "."
  cout << "   " << app << " + " << pabs << " + " << rel << " + " << ext << " + " << invalid << " + " << fin << " = " << endl;
  fs::path full = app / pabs / rel / ext / invalid / fin;
  cout << "   " << full << endl;

  // Test breaking up into parent path, filename, and extension
  cout << "3. parent, filename, & ext: " << endl;
  fs::path tmp = full;
  cout << "   full : " << tmp << endl;
  while (tmp.filename().string() != "")
  {
    // Note: parent_path removes double // and internal :/ -> :
    cout << "        = " << tmp.parent_path() << " + " << tmp.filename();
    if (tmp.has_extension())
      cout << " (" << tmp.extension() << ")" << endl;
    else
      cout << endl;
    tmp = tmp.parent_path();
  }
  cout << "        = " << tmp.parent_path() << " + " << tmp.filename();
  cout << endl; 

  cout << "4. Test wrong-platform path" << endl; 
  fs::path linux_path("/home/jdoe/test.txt"); 
  cout << "   " << linux_path << " = " << linux_path.string() << endl; 


#else

  // Test append. NOT smart about paths you provide from strings
  cout << "2. append: " << endl;
  fs::path app("/home/jdoe");
  fs::path pabs("/src/spacy path/");   // Trailing separator NOT removed.
  fs::path rel("/seps/");              // Leading separator also NOT removed. With this and previous, get 
                                       // double slash which is REMOVED in parent_path.
  fs::path ext("test.txt");
  //fs::path invalid("?");             // Only invalid char seems to be `/`
  fs::path fin("/a/");                 // If path ends in slash, filename is "."
  cout << "   " << app << " + " << pabs << " + " << rel << " + " << ext << " + " << fin << " = " << endl;
  fs::path full = app / pabs / rel / ext / fin;
  cout << "   " << full << endl;

  // Test breaking up into parent path, filename, and extension
  cout << "3. parent, filename, & ext: " << endl;
  fs::path tmp = full;
  cout << "   full : " << tmp << endl;
  while (tmp.filename().string() != "")
  {
    // Note: parent_path removes double \\ from path
    cout << "        = " << tmp.parent_path() << " + " << tmp.filename();
    if (tmp.has_extension())
      cout << " (" << tmp.extension() << ")" << endl;
    else
      cout << endl;
    tmp = tmp.parent_path();
  }
  cout << "        = " << tmp.parent_path() << " + " << tmp.filename();
  cout << endl; 

  cout << "4. Test wrong-platform path" << endl; 
  fs::path win_path("C:\\Users\\jdoe\\test.txt"); 
  cout << "   " << win_path << " = " << win_path.string() << endl; 

#endif

  
  return 0; 
}
