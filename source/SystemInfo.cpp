#include "utility/SystemInfo.h"

namespace utility
{

// Define operating system
#if   defined(_WIN32)
  const std::string SystemInfo::OS = "Windows";
#elif defined(__linux__) || defined(__unix__) || defined(__unix) || defined(unix)
  const std::string SystemInfo::OS = "Linux";
#elif defined(__APPLE__) || defined(__MACH__) 
  const std::string SystemInfo::OS = "MacOS";
#else
  const std::string SystemInfo::OS = "Unknown";
#endif


// Define compiler and version
#define STRINGIFY(x) #x
#define STRINGIFY_MACRO(x) STRINGIFY(x) 

#if   defined(__INTEL_COMPILER)
  const std::string SystemInfo::compiler = "Intel";
  const std::string SystemInfo::compiler_version = STRINGIFY_MACRO(__INTEL_COMPILER);
#elif defined(_MSC_VER)
  const std::string SystemInfo::compiler = "MSVC";
  //const std::string SystemInfo::compiler_version = STRINGIFY_MACRO(_MSC_VER);
  const std::string SystemInfo::compiler_version = STRINGIFY_MACRO(_MSC_FULL_VER); 
  // VER and FULL_VER encode the version without periods. To translate back, see
  // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#elif defined(__clang__) 
  const std::string SystemInfo::compiler = "Clang";
  const std::string SystemInfo::compiler_version = STRINGIFY_MACRO(__clang_major__.__clang_minor__.__clang_patchlevel__);
#elif defined(__GNUC__) 
  const std::string SystemInfo::compiler = "GNU";
  const std::string SystemInfo::compiler_version = STRINGIFY_MACRO(__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__);
#else
  const std::string SystemInfo::compiler = "Unknown";
  const std::string SystemInfo::compiler_version = "Unknown";
#endif

  std::string SystemInfo::get_compile_info()
  { 
    std::string compile_info = "Compiled on ";
    compile_info += OS + " with " + compiler + " " + compiler_version; 
    return (compile_info); 
  } 

}
