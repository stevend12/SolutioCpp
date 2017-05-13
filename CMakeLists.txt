# This is the CMakeLists file for the PhotonDemo program.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(SolutioMedPhys)

file(COPY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(Library)
add_subdirectory(Examples)
