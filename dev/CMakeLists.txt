add_executable(utility_dev source/main.cpp)
target_link_libraries(utility_dev PRIVATE Utility::Utility)

include(${CMAKE_SOURCE_DIR}/cmake/Utilities.cmake)
assign_source_group("utility" ${UTILITY_HEADER_LIST})
assign_source_group("utility" ${UTILITY_SOURCE_LIST})
