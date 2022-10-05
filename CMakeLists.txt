cmake_minimum_required( VERSION 3.17 )

cmake_policy( SET CMP0048 NEW )
project( UTILITY VERSION 1.0.0 LANGUAGES CXX )

#########################################
# Variables that configure this project #
#########################################

set(PUBLIC_MODULE_NAME "Utility")
set(MAIN_COMPONENT "utility")

set(SHEFF_FIRST_DEPENDS "")
set(SHEFF_SECOND_DEPENDS "")

# Third-party dependencies (length of all three lists must match).
# Since we cannot assume certain pattern for names, using multiple lists gives
# the flexibility needed for various packages.
set(SHEFF_THIRD_DEPENDS_name "json")
set(SHEFF_THIRD_DEPENDS_package "nlohmann_json")
set(SHEFF_THIRD_DEPENDS_target "nlohmann_json::nlohmann_json")

# External dependencies need to be found from the system (such as MPI)
# toggle_var indicates the variable name that controls whether the package is included.
# The entry can be "1" for packages that are always included.
set(SHEFF_EXTERNAL_DEPENDS_toggle_var "SHEFF_USE_MPI")
# The name of the package in the call find_package(<package name>)
set(SHEFF_EXTERNAL_DEPENDS_package "MPI")
# Any components to get from the package (use comma delimited for multiple components in the same package)
set(SHEFF_EXTERNAL_DEPENDS_components "CXX")
# The targets to link from the package (use comma delimited for multiple targets in the same package)
set(SHEFF_EXTERNAL_DEPENDS_targets "MPI::MPI_CXX")
# Extra compile definitions if the package is included
set(SHEFF_EXTERNAL_DEPENDS_compile_defs "HAVE_MPI")

macro(SPECIAL_TARGET_COMMANDS)
  # Use filesystem if available, otherwise use our hack.
  if( SHEFF_UseFilesystemHack )
  else()
    target_compile_definitions(${MAIN_COMPONENT} PUBLIC HAVE_FILESYSTEM )
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
        #target_link_libraries(${MAIN_COMPONENT} PUBLIC -static-libstdc++fs)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel" )
        # Trying the same thing with Intel, linked against newer GCC libs, but no luck
        target_link_libraries(${MAIN_COMPONENT} PUBLIC stdc++fs)
    elseif (MSVC)
        # This is to silence the error that MS will remove <experimental/filesystem> and replace
        # it with <filesystem>
        target_compile_definitions(${MAIN_COMPONENT} PUBLIC _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
    endif()
  endif()
endmacro()

# Options
option( SHEFF_BuildTests        "Build the unit tests." OFF )
option( SHEFF_BuildPython       "Build Python SWIG module" OFF )
option( SHEFF_BuildFortran      "Build Fortran SWIG module" OFF )
option( SHEFF_USE_MPI           "Indicates if the MPI routines should be included in the build." OFF )
option( SHEFF_UseFilesystemHack "Use if compiler has included <experimental/filesystem>" OFF )

# For debugging, print generator, compiler, and options
message( STATUS "Gen: ${CMAKE_GENERATOR}, CXX: ${CMAKE_CXX_COMPILER_ID}, Mode: ${CMAKE_BUILD_TYPE}" )
message( STATUS "Python Bindings: ${SHEFF_BuildPython}, Fortran Bindings: ${SHEFF_BuildPython}" )
message( STATUS "Tests: ${SHEFF_BuildTests}" )
message( STATUS "FS HACK: ${SHEFF_UseFilesystemHack}" )

include("$ENV{SHEFF_TOOLS}/cmake/SHEFFModuleMain.cmake")
