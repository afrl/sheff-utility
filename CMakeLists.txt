cmake_minimum_required( VERSION 3.8 )

cmake_policy( SET CMP0048 NEW )
project( UTILITY VERSION 1.0.0 LANGUAGES CXX )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 6.0
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
        message( FATAL_ERROR "GCC version must be at least 6.0! (Found: ${CMAKE_CXX_COMPILER_VERSION})" )
    endif()
endif()

# Set package toolchain location for Linux. To enable efficient switching between
# toolchains for cross-compiling, we modify the location where CMake looks for 
# packages using the find_package command by symbolically linking the default location,
# $HOME/.cmake/packages, to a similar location with the folder name given by the 
# compiler family. 
if (UNIX)
    execute_process(COMMAND "mkdir" "-p" "$ENV{HOME}/.cmake/${CMAKE_CXX_COMPILER_ID}"
      RESULT_VARIABLE toolchain_res OUTPUT_VARIABLE toolchain_out)
    if (toolchain_res)
        message(ERROR "Couldn't create toolchain dir: $ENV{HOME}/.cmake/${CMAKE_CXX_COMPILER_ID} ${toolchain_out}")
    endif ()
    execute_process(COMMAND "ln" "-sfn" "$ENV{HOME}/.cmake/${CMAKE_CXX_COMPILER_ID}" "$ENV{HOME}/.cmake/packages"
      RESULT_VARIABLE toolchain_res OUTPUT_VARIABLE toolchain_out)
    if (toolchain_res)
        message(ERROR "Couldn't create toolchain link: $ENV{HOME}/.cmake/packages ${toolchain_out}")
    endif ()
endif()


# Options
option( UTILITY_BuildTests        "Build the unit tests." OFF )
option( UTILITY_USE_MPI "Indicates if the MPI routines should be included in the build." OFF )
option( UTILITY_UseFilesystemHack "Use if compiler has included <experimental/filesystem>" OFF )

# For debugging, print generator, compiler, and options
message( STATUS "Gen: ${CMAKE_GENERATOR}, CXX: ${CMAKE_CXX_COMPILER_ID}, Mode: ${CMAKE_BUILD_TYPE}" )
message( STATUS "Tests: ${UTILITY_BuildTests}, FS HACK: ${UTILITY_UseFilesystemHack}" )

# Add cmake dir to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

##################
# Checking input #
##################

include(${CMAKE_SOURCE_DIR}/cmake/CommonInputChecks.cmake)

########################
# Declare dependencies #
########################


find_package(SecondPartyMath CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../second_party/second_party_math/install_dir" "${CMAKE_SOURCE_DIR}/../second_party_math/install_dir" "${CMAKE_SOURCE_DIR}/modules/second_party/second_party_math/install_dir")
find_package(nlohmann_json 3.2.0 CONFIG REQUIRED HINTS 
  "${CMAKE_SOURCE_DIR}/../third_party/json_install_dir" "${CMAKE_SOURCE_DIR}/../../third_party/json_install_dir" "${CMAKE_SOURCE_DIR}/modules/third_party/json_install_dir")


if( UTILITY_USE_MPI )
    find_package( MPI REQUIRED )
endif()

################################
# Setting up UTILITY target #
################################

# Set the variables for UTILITY
set( UTILITY_INCLUDE     ${CMAKE_SOURCE_DIR}/include )
set( UTILITY_SRC         ${CMAKE_SOURCE_DIR}/source )

file( GLOB_RECURSE UTILITY_HEADER_LIST "${UTILITY_INCLUDE}/*.h" )
file( GLOB_RECURSE UTILITY_SOURCE_LIST "${UTILITY_SRC}/*.h" "${UTILITY_SRC}/*.cpp" )
    
# Create target
add_library(utility STATIC)
# Add the header includes and headers to the library
target_include_directories(utility PUBLIC
    $<BUILD_INTERFACE:${UTILITY_INCLUDE}>
    $<INSTALL_INTERFACE:include>)
# Add headers in the include folder as PUBLIC so downstream
# targets have access to them
target_sources(utility PUBLIC ${UTILITY_HEADER_LIST})
# Add headers and sources in the source folder as PRIVATE as they
# will not be needed downstream
target_sources(utility PRIVATE ${UTILITY_SOURCE_LIST})
# Add an alias so that a library can be used inside the build tree, eg when testing
add_library(Utility::Utility ALIAS utility)
# Add C++ 14 flag
target_compile_features(utility PUBLIC cxx_std_14)

# Use filesystem if available, otherwise use our hack.
if( UTILITY_UseFilesystemHack )
else()
  target_compile_definitions(utility PUBLIC HAVE_FILESYSTEM )
  if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
      target_link_libraries( utility PUBLIC libstdc++fs.a )
  elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel" )
      # Trying the same thing with Intel, linked against newer GCC libs, but no luck
      target_link_libraries( utility PUBLIC libstdc++fs.a )
  endif()
  if (MSVC)
      # This is to silence the error that MS will remove <experimental/filesystem> and replace
      # it with <filesystem>
      target_compile_definitions(utility PUBLIC _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
  endif()
endif()

# Link dependencies
target_link_libraries(utility PUBLIC SecondPartyMath::SecondPartyMath)
target_link_libraries(utility PUBLIC nlohmann_json::nlohmann_json)


if(UTILITY_USE_MPI AND MPI_FOUND)
    target_compile_definitions(utility PUBLIC HAVE_MPI)
    target_include_directories(utility PUBLIC ${MPI_CXX_INCLUDE_PATH})
    target_link_libraries(utility PUBLIC ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})
    target_compile_options(utility PUBLIC ${MPI_CXX_COMPILE_FLAGS})
endif()

# Assign source groups
include(${CMAKE_SOURCE_DIR}/cmake/Utilities.cmake)
assign_source_group("utility" ${UTILITY_HEADER_LIST})
assign_source_group("utility" ${UTILITY_SOURCE_LIST})
assign_source_group_SecondPartyMath()


#################################
# Add unit test targets #
#################################

# Build tests
if(UTILITY_BuildTests)
    include(CTest)
    enable_testing()
    add_subdirectory(unit_tests)
endif()

################
# Installation #
################

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Utility)

# Install the utility target (library and headers)
install(
    TARGETS utility
    EXPORT UtilityTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)
# This is required so that the exported target has the name Utility and not utility
set_target_properties(utility PROPERTIES EXPORT_NAME Utility)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/UtilityConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create the package config file for find_package(Utility)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/UtilityConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/UtilityConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config and config version files
# Note: if you write custom find modules, add them here
# Refer to https://github.com/pabloariasal/modern-cmake-sample/blob/cfda7b51ff915409fd61d4f2e75b63032bedb182/libjsonutils/CMakeLists.txt
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/UtilityConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/UtilityConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/UtilityTargets.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

###########
# Exports #
###########

# Export just the utility target (not tests, etc.)
export(
    TARGETS utility
    FILE ${CMAKE_CURRENT_BINARY_DIR}/UtilityTargets.cmake
    NAMESPACE Utility::
)

# Register package in user's package registry
export(PACKAGE Utility)
