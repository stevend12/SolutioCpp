# This is the CMakeLists file for the PhotonDemo program.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

add_subdirectory(PhotonDemo)
add_subdirectory(RayCtDemo)
