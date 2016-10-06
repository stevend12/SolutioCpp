# This is the CMakeLists file for the PhotonDemo program.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(solutio)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Add sources and headers
set(SOURCE
  ${SOURCE}
  # General physics
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistPad.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistPad.cpp
  # Imaging
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/Tasmip.hpp
  # Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DataInterpolation.hpp
)

add_library(solutio STATIC ${SOURCE})
