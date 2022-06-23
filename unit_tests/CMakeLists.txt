include("$ENV{SHEFF_TOOLS}/cmake/IncludeSHEFF.cmake")

# Add all test as serial tests
file(GLOB_RECURSE _unit_tests "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
foreach(_test ${_unit_tests})
  add_serial_test(${MAIN_COMPONENT} ${PUBLIC_MODULE_NAME}::${MAIN_COMPONENT} ${_test})
endforeach()

# Add any MPI tests here:
if(MPI_FOUND)
  message(STATUS "Building MPI tests for ${COMPONENT}.")
  add_mpi_test(${MAIN_COMPONENT} ${PUBLIC_MODULE_NAME}::${MAIN_COMPONENT} "./source/test_MPIUtils.cpp" 2)
  add_mpi_test(${MAIN_COMPONENT} ${PUBLIC_MODULE_NAME}::${MAIN_COMPONENT} "./source/test_MPIUtils.cpp" 4)
  add_mpi_test(${MAIN_COMPONENT} ${PUBLIC_MODULE_NAME}::${MAIN_COMPONENT} "./source/test_MPIUtils.cpp" 6)
endif ()# End MPI tests

assign_source_groups_sheff()
