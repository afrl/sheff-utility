cmake_minimum_required(VERSION 3.8)

cmake_policy(SET CMP0048 NEW)
project(UTILITY VERSION 1.0.0 LANGUAGES CXX)

# Options
option(UTILITY_BuildDev "Build the project in the dev folder. (intended for debugging this project by devs)" OFF)
option(UTILITY_BuildTests "Build the unit tests." OFF)
option(UTILITY_HaveFilesystem "Use if compiler has included <experimental/filesystem>" ON) 

# For debugging, print generator, compiler, and options
message(STATUS "Gen: ${CMAKE_GENERATOR}, CXX: ${CMAKE_CXX_COMPILER_ID}, Mode: ${CMAKE_BUILD_TYPE}")
message(STATUS "Dev: ${UTILITY_BuildDev}, Tests: ${UTILITY_BuildTests}, FS: ${UTILITY_HaveFilesystem}")

##################
# Checking input #
##################

include(${CMAKE_SOURCE_DIR}/cmake/CommonInputChecks.cmake)

########################
# Declare dependencies #
########################




################################
# Setting up UTILITY target #
################################

# Set the variables for UTILITY
set(UTILITY_INCLUDE ${CMAKE_SOURCE_DIR}/include)
set(UTILITY_INCLUDE_SUB ${UTILITY_INCLUDE}/utility)
set(UTILITY_SRC ${CMAKE_SOURCE_DIR}/source)
file (GLOB_RECURSE UTILITY_HEADER_LIST "${UTILITY_INCLUDE}/*.h")
file (GLOB_RECURSE UTILITY_SOURCE_LIST "${UTILITY_SRC}/*.h" "${UTILITY_SRC}/*.cpp")
    
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
# Use filesystem if available
if (UTILITY_HaveFilesystem)
  add_definitions(-DHAVE_FILESYSTEM)
  # Add flag to link with experimental/filesystem if GNU. This requires GCC >= 5.3.
  if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
      target_link_libraries(utility libstdc++fs.a)
  endif()
else()
  remove_definitions(-DHAVE_FILESYSTEM)
endif()
# Link dependencies

# Modify the properties for one source in particular due to the source file requiring too
# larger of an object on MSVC
if (MSVC)
    set_source_files_properties(${UTILITY_SRC}/SpatialTemporalValue.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

# Assign source groups
include(${CMAKE_SOURCE_DIR}/cmake/Utilities.cmake)
assign_source_group("utility" ${UTILITY_HEADER_LIST})
assign_source_group("utility" ${UTILITY_SOURCE_LIST})


#################################
# Add dev and unit test targets #
#################################

# Build dev
if(UTILITY_BuildDev)
    add_subdirectory(dev)
endif()
    
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

# Export just the utility target (not the dev, tests, etc.)
export(
    TARGETS utility
    FILE ${CMAKE_CURRENT_BINARY_DIR}/UtilityTargets.cmake
    NAMESPACE Utility::
)

# Register package in user's package registry
export(PACKAGE Utility)

