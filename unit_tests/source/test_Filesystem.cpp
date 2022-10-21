/// Unit test for Filesystem.h

#include<iostream>

//#include "utility/Filesystem.h"
#include <filesystem>
namespace fs = std::filesystem;

using namespace std; 

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

  cout << "4. Test assign from string, exists, is dir/abs, abs: " << endl; 
  tmp = "test.txt"; 
  cout << "   exists: " << fs::exists(tmp) << endl; 
  cout << "   is dir: " << fs::is_directory(tmp) << endl; 
  cout << "   is abs: " << tmp.is_absolute() << endl; 
  cout << "   abs   : " << fs::absolute(tmp) << endl; 
  cout << endl; 

  cout << "5. Test wrong-platform path" << endl; 
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

  cout << "4. Test assign from string, exists, is dir/abs, abs: " << endl; 
  tmp = "test.txt"; 
  cout << "   exists: " << fs::exists(tmp) << endl; 
  cout << "   is dir: " << fs::is_directory(tmp) << endl; 
  cout << "   is abs: " << tmp.is_absolute() << endl; 
  cout << "   abs   : " << fs::absolute(tmp) << endl; 
  cout << endl; 

  cout << "5. Test wrong-platform path" << endl; 
  fs::path win_path("C:\\Users\\jdoe\\test.txt"); 
  cout << "   " << win_path << " = " << win_path.string() << endl; 

#endif
  
  // directory_iterator is know to causes problems on some versions of GCC, so we test it
  cout << "6. Iterating through current directory:" << endl;
  for (auto const& dir_entry : fs::directory_iterator(cwd)) 
  {
      cout << "   " << dir_entry.path() << '\n';
  } 

  return 0; 
}
