#include "utility/Filesystem.h"

namespace std {
namespace experimental {
namespace filesystem {

#ifdef _GLIBCXX_FILESYSTEM_IS_WINDOWS 
    const char path::separator = '\\';
#else
    const char path::separator = '/';
#endif

    path current_path()
    {
        char cwd[FILENAME_MAX];   // Linux 4096, Win: 260 !!
        if (!GetCurrentDir(cwd, FILENAME_MAX)) {
            std::cout << "current_path(): error!\n";
        }

        std::string cwd_str( cwd );
        return path( cwd_str );
    };
}}} // end namespaces
