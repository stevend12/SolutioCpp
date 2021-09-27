# This is the CMakeLists file for the PhysicsDemo program.
cmake_minimum_required(VERSION 3.0.0)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(PhysicsDemo)

include_directories(${LIB_INCLUDE_DIR})
add_executable(PhysicsDemo PhysicsDemo.cpp)
target_link_libraries(PhysicsDemo solutio)
configure_file(PhysicsDemo.m ${CMAKE_CURRENT_BINARY_DIR}/PhysicsDemo.m COPYONLY)
