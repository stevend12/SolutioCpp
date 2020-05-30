# This is the CMakeLists file for the PhotonDemo program.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(RayCtDemo)

include_directories(${LIB_INCLUDE_DIR})
add_executable(RayCtDemo RayCtDemo.cpp)
target_link_libraries(RayCtDemo solutio -fopenmp fftw3 fftw3_threads)
