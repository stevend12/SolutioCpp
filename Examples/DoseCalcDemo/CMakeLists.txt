# This is the CMakeLists file for the DoseCalcDemo program.
cmake_minimum_required(VERSION 3.0.0)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(DoseCalcDemo)

include_directories(${LIB_INCLUDE_DIR})
add_executable(DoseCalcDemo DoseCalcDemo.cpp)
target_link_libraries(DoseCalcDemo solutio)
#configure_file(DoseCalcDemo.m ${CMAKE_CURRENT_BINARY_DIR}/DoseCalcDemo.m COPYONLY)
