#pragma once
/// Defines system/compiler-dependent information

#include <string>

namespace utility
{
   class SystemInfo
   {
     public:

       // Structors

       SystemInfo (){}; 

       ~SystemInfo (){}; 

       /// Get compile info. Return a string with information about the platform,
       /// compiler, and additional optional libraries (e.g., MKL, PETSc) used to compile.
       std::string get_compile_info(); 

       /// Operating system
       static const std::string OS;

       /// Compiler
       static const std::string compiler; 

       /// Compiler Version
       static const std::string compiler_version; 

       ///TODO Define platform-specific commands as needed: move, delete, copy, etc.
   
       /// Tell if compiled with petsc, mpi, mkl, etc.
   }; 

}
