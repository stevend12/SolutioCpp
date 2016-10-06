# This is the CMakeLists file for the PhotonDemo program.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(PhotonDemo)

include_directories(${LIB_INCLUDE_DIR})
add_executable(PhotonDemo PhotonDemo.cpp)
target_link_libraries(PhotonDemo solutio)
