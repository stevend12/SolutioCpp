# This is the CMakeLists file for the AbsoluteDoseCalibrationDemo program.
cmake_minimum_required(VERSION 3.0.0)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(AbsoluteDoseCalibrationDemo)

include_directories(${LIB_INCLUDE_DIR})
add_executable(AbsoluteDoseCalibrationDemo AbsoluteDoseCalibrationDemo.cpp)
target_link_libraries(AbsoluteDoseCalibrationDemo solutio)
configure_file(AbsoluteDoseCalibrationDemo.m
  ${CMAKE_CURRENT_BINARY_DIR}/AbsoluteDoseCalibrationDemo.m COPYONLY)
